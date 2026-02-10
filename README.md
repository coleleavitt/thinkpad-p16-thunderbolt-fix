# ThinkPad P16 Gen 3 — Thunderbolt GPE 0x6E Interrupt Storm Fix

Reverse engineering and fix for a firmware bug in the Lenovo ThinkPad P16 Gen 3 (21RQ) that causes a GPE 0x6E interrupt storm, pegging the CPU at ~90°C and wasting ~15% CPU on ACPI interrupt processing.

## The Problem

The Barlow Ridge Thunderbolt 4 controller (Intel 8086:5780) holds GPIO pin P1WG asserted whenever it is powered on. The PCH routes P1WG to GPE 0x6E as a **level-triggered** interrupt. The DSDT `_L6E` handler clears the GPIO status bits, but because P1WG is a hardware presence signal that cannot be de-asserted in software, the GPE immediately re-fires — producing ~400 interrupts/second (164,000+ events before the kernel auto-disables it).

**Symptoms:**
- CPU package temperature 88–91°C at idle
- `cat /sys/firmware/acpi/interrupts/gpe6E` shows massive event count
- `dmesg` shows: `ACPI: EC: interrupt storm detected, GPE 0x6E disabled`
- ~15% CPU consumed by `kworker` threads processing ACPI events

## Root Cause Chain

Traced across **6 UEFI firmware modules** using IDA Pro 9.2:

```
BIOS Setup Variable "Setup" offset 0x76D = 1 (DTbt Enable, default)
    │
    ▼
AdvancedAcpiDxe.efi
    │  Copies Setup[0x76D] → GNVS offset 831 = 1
    ▼
DTbtDxe.efi
    │  Reads GNVS+831 = 1 → DTbt enabled
    │  Patches DTbt SSDT (signature "DtbStdt") with NVS pointer + enable flag
    │  Writes PCI register 0x54C = 0x63 on Barlow Ridge (Force Power ON)
    ▼
Barlow Ridge 8086:5780 powers up
    │  Asserts GPIO P1WG (hardware presence signal, CANNOT be cleared)
    ▼
PCH routes P1WG → GPE 0x6E (level-triggered)
    │
    ▼
PchInitDxe.efi
    │  Sets PCHA = 1 (PCH ACPI Enable) based on PCI 00:1F.0 DID = 0xAE10
    ▼
DSDT _L6E handler (line 43853)
    │  Checks PCHA == 1 → calls PL6E() in SSDT27
    ▼
SSDT27 PL6E() (line 1036)
    │  Clears GPIO status (CGPI) and GPE status (S200 bit 14)
    │  BUG: P1WG is level-triggered and still asserted
    │  → GPE immediately re-fires → INTERRUPT STORM (~400/sec)
```

## The Fix

Two layers (belt-and-suspenders):

### Layer 1: Kernel Parameter
```
acpi_mask_gpe=0x6E
```
Add to `GRUB_CMDLINE_LINUX` in `/etc/default/grub`. The kernel masks GPE 0x6E before any handlers run.

### Layer 2: Custom SSDT Overlay
A custom ACPI SSDT table (`fix/ssdt-tbt-fix.dsl`) that disables GPE 0x6E at the hardware register level during ACPI `_INI`. This clears bit 14 in the GPE0_EN register block (ACPI base + 0x7C), preventing the GPE from ever being enabled.

```bash
cd fix/
chmod +x build.sh
./build.sh
```

See `fix/build.sh` for full installation instructions.

**Neither fix affects Thunderbolt functionality.** Linux's `thunderbolt` driver uses MSI-X interrupts exclusively; the ACPI GPE path is a legacy Windows mechanism unused on Linux.

## Hardware Topology

```
00:07.0  Meteor Lake TBT4 Root Port [8086:7ec4]
  └─ 88:00.0  Barlow Ridge Top Bridge [8086:5780] (rev 84)
       ├─ 89:00.0  Bridge [8086:5780]
       ├─ 89:01.0  Bridge [8086:5780]
       ├─ 89:02.0  Bridge [8086:5780]
       ├─ 89:03.0  Bridge [8086:5780]
       └─ 8a:00.0  NHI [8086:5781] (rev 84)
            BAR0 = 0x4FFFF00000 (256K MMIO)
            BAR2 = 0x4FFFF40000 (4K MSI-X, 16 vectors)
            Driver: thunderbolt
```

## NHI Register Dump (Key Findings)

Live register values from the investigation kernel module (`nhi_regdump.c`):

| Register | Address | Value | Meaning |
|----------|---------|-------|---------|
| VS_CAP_9 (FW_READY) | PCI 0xC8 | 0x0010291F | FW_READY=0, firmware not running |
| VS_CAP_19 (PCIe2TBT) | PCI 0xF0 | 0x00000001 | Stuck mailbox VALID bit |
| VS_CAP_22 (FORCE_POWER) | PCI 0xFC | 0x00000000 | Force power cleared (by kernel driver) |
| REG_FW_STS | BAR0+0x39944 | 0x00000000 | No firmware state |
| REG_DMA_MISC | BAR0+0x39864 | 0xFF900016 | INT_AUTO_CLEAR=1 |
| RING_INT_EN[0] | BAR0+0x38200 | 0x00001001 | Rings 0,12 enabled |

**Experiment:** Cleared VS_CAP_19 and re-enabled GPE 0x6E → storm immediately resumed at ~398/sec. Proves P1WG is held HIGH by hardware regardless of mailbox or force-power state.

## Repository Structure

```
fix/
  ssdt-tbt-fix.dsl          # Custom SSDT overlay source (ASL)
  ssdt-tbt-fix.aml          # Compiled AML binary (170 bytes)
  build.sh                  # Build + install script

investigation/
  modules/
    nhi_regdump.c            # Dumps Barlow Ridge NHI BAR0 MMIO registers
    read_p1wg3.c             # Reads TBT GPIO pad values (P1WG/P1PG/P1RG) from GNVS
    read_pcha_mod.c          # Reads PCHA value from PCH NVS (PNVA) region
    Makefile                 # Build all investigation modules

analysis/
  firmware/
    DTbtDxe_analysis.md      # Discrete Thunderbolt DXE driver (22 functions, 10 GUIDs)
    ITbtDxe_analysis.md      # Integrated Thunderbolt DXE driver (18 functions, 14 GUIDs)
    AdvancedAcpiDxe_analysis.md  # Master ACPI NVS builder (91 functions, 27 GUIDs)
  decompiled/
    DTbtDxe_decompiled.c     # Full Hex-Rays decompilation (774 lines)
    ITbtDxe_decompiled.c     # Full Hex-Rays decompilation (696 lines)
```

## Investigation Modules

Out-of-tree kernel modules used to probe live hardware state. They load, dump registers to `dmesg`, and immediately unload (return `-EAGAIN`).

```bash
cd investigation/modules/
make
sudo insmod nhi_regdump.ko    # Dump NHI registers
sudo insmod read_p1wg3.ko     # Read GPIO pad values
sudo insmod read_pcha_mod.ko  # Read PCHA from PCH NVS
dmesg | tail -40
```

**Note:** These modules use hardcoded GNVS/PNVA physical addresses specific to this system's BIOS version (N4FET30W 1.11). They will need adjustment for other firmware versions.

## System Details

| Component | Value |
|-----------|-------|
| **Model** | Lenovo ThinkPad P16 Gen 3 (21RQ002XUS) |
| **CPU** | Intel Arrow Lake-S |
| **BIOS** | N4FET30W (1.11), Phoenix SecureCore, dated 10/03/2025 |
| **EC** | Firmware rev 1.10 |
| **PCH** | Device ID 0xAE10 (CNL-H classification in firmware) |
| **Thunderbolt** | Barlow Ridge 80G (8086:5780/5781/5782, TBT4/USB4) |
| **OS** | Gentoo Linux, kernel 6.19.0-rc8, niri Wayland compositor |
| **Tools** | IDA Pro 9.2 + Hex-Rays, UEFITool NE, iasl, ifdtool |

## Affected Systems

Any Lenovo ThinkPad (or other OEM) with:
- Intel Meteor Lake / Arrow Lake platform
- Barlow Ridge (8086:5780) discrete Thunderbolt 4 controller
- Phoenix SecureCore BIOS with DTbtDxe.efi

The bug is in the DSDT/SSDT GPE handler logic (`_L6E` / `PL6E`), not in the kernel. Lenovo needs to update the firmware to either:
1. Disable GPE 0x6E in the `_L6E` handler after the first wake event
2. Switch P1WG to edge-triggered instead of level-triggered
3. Add a proper interrupt mask in PL6E() before clearing status

## License

GPL-2.0 (kernel modules), ACPI overlay is public domain.

## Credits

Analysis performed February 2026 using IDA Pro 9.2 MCP integration for automated firmware reverse engineering.
