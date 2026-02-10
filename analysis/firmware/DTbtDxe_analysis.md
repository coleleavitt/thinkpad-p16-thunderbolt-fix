# DTbtDxe.efi — Complete Reverse Engineering Analysis

**Target:** DTbtDxe.efi (Discrete Thunderbolt DXE Driver)
**System:** Lenovo ThinkPad P16 Gen3 (21RQ002XUS), Arrow Lake-S
**BIOS:** N4FET30W (1.11), Phoenix SecureCore
**Module GUID:** `180D765F-C489-4F7A-8A3F-596018499EAF`
**Binary:** PE32+ x86-64, 22 functions, entry at 0x3D0
**Tool:** IDA Pro 9.2 via MCP JSON-RPC, Hex-Rays Decompiler 9.3

---

## Executive Summary

DTbtDxe.efi is a UEFI DXE driver responsible for initializing Discrete Thunderbolt
(DTbt) controllers during the boot process. It reads configuration from PEI-phase HOBs
and NVRAM setup variables, programs Thunderbolt PCI config registers, patches ACPI SSDT
tables, and installs NVS area protocols for runtime ACPI/SMM communication.

**This driver is the firmware-side root cause of the GPE 0x6E interrupt storm** — it
enables the ACPI `_L6E` handler by patching the DTbt SSDT table and powers up the
Thunderbolt controller, which then drives GPIO pin P1WG. The PCH routes P1WG to GPE
0x6E, and the level-triggered handler in SSDT27 fails to properly acknowledge the GPIO
source, causing a re-fire loop at ~400/sec.

---

## GUID Table (All 10 GUIDs Identified)

| Address  | Symbolic Name                        | GUID                                   | Type          | Role        |
|----------|--------------------------------------|----------------------------------------|---------------|-------------|
| `0x12F8` | `gEfiAcpiTableProtocolGuid`          | `FFE06BDD-6107-46A6-7BB2-5A9C7EC5275C` | UEFI Protocol | CONSUMES    |
| `0x12C8` | `gEfiFirmwareVolume2ProtocolGuid`    | `220E73B6-6BDB-4413-8405-B974B108619A` | PI Protocol   | CONSUMES    |
| `0x1318` | `gEfiHobListGuid`                    | `7739F24C-93D7-11D4-9A3A-0090273FC14D` | Config Table  | CONSUMES    |
| `0x12D8` | `gEfiSetupVariableGuid`              | `EC87D643-EBA4-4BB5-A1E5-3F3E36B20DA9` | NVRAM GUID    | CONSUMES    |
| `0x1348` | `gEfiEndOfDxeEventGroupGuid`         | `02CE967A-DD7E-4FFC-9EE7-810CF0470880` | Event Group   | CONSUMES    |
| `0x12E8` | `gDTbtInfoHobGuid`                   | `C2D628A1-DD7A-4865-A8EC-8961F357F863` | HOB GUID      | CONSUMES    |
| `0x1308` | `gDTbtNvsAreaProtocolGuid`           | `D7778B4C-85B5-479B-A883-80BD23EB1C48` | Protocol      | **PRODUCES** |
| `0x1338` | `gDxeDisableDTbtBmeProtocolGuid`     | `29627704-F178-442C-8E1B-FC29943A4659` | Protocol      | PRODUCES    |
| `0x1368` | `gDxeDTbtPolicyProtocolGuid`         | `714AEDBD-FBF6-4CF9-B5C7-B93ABD21E0CD` | Protocol      | CONSUMES    |
| `0x1358` | `gDTbtDxeModuleGuid` (FILE_GUID)     | `180D765F-C489-4F7A-8A3F-596018499EAF` | Module GUID   | IDENTITY    |

**Source:** Standard GUIDs from `MdePkg`/`MdeModulePkg` (edk2). Intel DTbt GUIDs from
`Intel/ElkhartLakePlatSamplePkg/PlatformPkg.dec` in the Intel Elkhart Lake reference code.

---

## Function Map (22 Functions, Fully Decompiled)

### Core Driver Logic

| Address | Name                             | Purpose                                                             |
|---------|----------------------------------|---------------------------------------------------------------------|
| `0x3D0` | `_ModuleEntryPoint`              | Standard DXE entry; saves gRT/gBS/gST, calls GetHobData + DriverInit |
| `0xA40` | `DTbt_DriverInit`                | **Main init** — reads config, patches ACPI, registers callbacks      |
| `0x408` | `DTbt_PatchAcpiSsdt`            | Finds DTbt SSDT in FV, patches 'DTDV' marker with policy data       |
| `0x66C` | `DTbt_PopulatePolicyFromConfig`  | Copies DTbt HOB data into NVS area policy structure                  |
| `0x774` | `DTbt_ProgramPciConfig`          | **EndOfDxe callback** — programs TBT PCI register 0x54C (Force Power)|
| `0x5C4` | `DTbt_ReadyToBootCallback`       | **ReadyToBoot callback** — enumerates TBT PCI bus on each port       |

### PCI Bus Enumeration

| Address | Name                               | Purpose                                                            |
|---------|------------------------------------|--------------------------------------------------------------------|
| `0xF0C` | `DTbt_EnumeratePciBus`             | Recursive PCI bus walk — scans bridges, finds TBT topology         |
| `0xC98` | `DTbt_IsKnownTbtDeviceId`          | Checks PCI device ID against 11 known TBT controller IDs          |
| `0xCEC` | `DTbt_PollPciRegister`             | Polls PCI register with 500ms timeout (0x1F4 × 1ms stalls)        |
| `0xE58` | `DTbt_BuildPciAddress`             | Converts Bus:Dev:Func tuple to PCI ECAM address                   |
| `0xE98` | `DTbt_ClearPciMasterAbort`         | Reads PCI Status, clears Master Abort bit (bit 2)                  |
| `0x11F8`| `DTbt_FindPciExpressCapability`    | Walks PCI capability list looking for PCI Express cap (ID=0x10)    |

### HOB / Memory Helpers

| Address | Name                    | Purpose                                                          |
|---------|-------------------------|------------------------------------------------------------------|
| `0xDA4` | `DTbt_GetHobData`       | Searches SystemTable->ConfigurationTable for gEfiHobListGuid     |
| `0xE10` | `DTbt_FindHobByGuid`    | Walks HOB list for type=4 (GUID Extension) matching given GUID   |
| `0xD74` | `DTbt_AllocatePool`     | Wrapper for gBS->AllocatePool(EfiRuntimeServicesData, size)      |

### Utility Functions

| Address | Name                   | Purpose                                             |
|---------|------------------------|-----------------------------------------------------|
| `0x2A0` | `efi_memset`           | Standard memset                                     |
| `0x300` | `efi_zeromem`          | Optimized zero-fill (qword then byte residual)      |
| `0x360` | `cpu_pause`            | `_mm_pause()` — yield during spin-wait              |
| `0x370` | `read_tsc`             | `__rdtsc()` — timestamp counter read                |
| `0x380` | `cpuid_wrapper`        | CPUID instruction wrapper (leaf, eax/ebx/ecx/edx)   |
| `0x1140`| `DTbt_Stall1ms`        | 1ms stall using TSC frequency calibration            |
| `0x1194`| `DTbt_GetTscFreqMhz`   | CPUID leaf 0x15 TSC frequency calculation            |

---

## Global Variables

| Address    | Name                  | Type    | Purpose                                        |
|------------|-----------------------|---------|-------------------------------------------------|
| `0x13C0`   | `gRT`                 | PTR     | EFI_RUNTIME_SERVICES pointer                   |
| `0x13D0`   | `gBS`                 | PTR     | EFI_BOOT_SERVICES pointer                      |
| `0x13C8`   | `gST`                 | PTR     | EFI_SYSTEM_TABLE pointer                       |
| `0x13D8`   | `gHobData`            | PTR     | Cached HOB list pointer                        |
| `0x13B0`   | `gDTbtConfig`         | PTR     | DTBT_INFO_HOB data (from PEI phase)            |
| `0x13B8`   | `gDTbtEnabled_Port0`  | BYTE    | DTbt port 0 enable flag                        |
| `0x13B9`   | `gDTbtEnabled_Port1`  | BYTE    | DTbt port 1 enable flag (1 or 2)               |
| `0x13E0`   | `gDTbtPolicy`         | PTR     | DTBT_NVS_AREA — 42-byte policy/NVS structure   |
| `0x1380`   | `wszSetup`            | WCHAR[] | L"Setup" — NVRAM variable name                 |

---

## Known Thunderbolt Device IDs

DTbt_IsKnownTbtDeviceId checks against these Intel Thunderbolt controller PCI Device IDs:

| Decimal | Hex    | Codename                          |
|---------|--------|-----------------------------------|
| 4403    | 0x1133 | Alpine Ridge LP 2C (TBT3)        |
| 4406    | 0x1136 | Alpine Ridge LP 4C (TBT3)        |
| 5494    | 0x1576 | Alpine Ridge 2C (TBT3)           |
| 5496    | 0x1578 | Alpine Ridge 4C (TBT3)           |
| 5568    | 0x15C0 | Alpine Ridge C 2C (TBT3)         |
| 5587    | 0x15D3 | Titan Ridge 2C (TBT3)            |
| 5594    | 0x15DA | Titan Ridge LP 2C (TBT3)         |
| 5607    | 0x15E7 | Maple Ridge 2C (TBT3/4)          |
| 5610    | 0x15EA | Maple Ridge 4C (TBT3/4)          |
| **22400** | **0x5780** | **Barlow Ridge 80G (TBT4/5)** ← **This system** |
| 22403   | 0x5783 | Barlow Ridge variant (TBT4/5)    |

---

## Driver Execution Flow

### Phase 1: Initialization (DTbt_DriverInit, called from entry point)

```
1. DTbt_GetHobData() → Find HOB list from SystemTable->ConfigurationTable
2. DTbt_FindHobByGuid(gDTbtInfoHobGuid) → Get DTbt config from PEI phase
   - Reads port enable flags from offsets +28 and +60
   - Reads Bus/Dev/Func from offsets +52/+53/+54 (per port, 32-byte stride)
3. gBS->AllocatePool(42 bytes) → Allocate NVS area (gDTbtPolicy)
4. DTbt_PatchAcpiSsdt() → Patch DTbt SSDT table [see Phase 2]
5. DTbt_PopulatePolicyFromConfig() → Fill NVS area from DTbt config + policy
6. gBS->InstallProtocolInterface(gDTbtNvsAreaProtocolGuid, gDTbtPolicy)
7. gBS->CreateEventEx(EndOfDxe, DTbt_ProgramPciConfig) → Register callback
8. gRT->GetVariable(L"Setup", gEfiSetupVariableGuid) → Read 3316-byte Setup var
   - Check Setup[1843], Setup[2001], Setup[2542], Setup[51] (enable conditions)
   - Setup[29] → Override gDTbtEnabled_Port0
   - Setup[2830] == 2 → gDTbtEnabled_Port1 = 2, else 1
9. gBS->LocateProtocol(gDxeDTbtPolicyProtocolGuid) → Check platform policy
10. If policy->enabled == 1:
    - gBS->CreateEventEx(ReadyToBoot, DTbt_ReadyToBootCallback)
    - gBS->InstallProtocolInterface(gDxeDisableDTbtBmeProtocolGuid, sub_5C4)
```

### Phase 2: ACPI SSDT Patching (DTbt_PatchAcpiSsdt)

```
1. gBS->LocateProtocol(gEfiAcpiTableProtocolGuid) → Get ACPI protocol
2. gBS->LocateHandleBuffer(gEfiFirmwareVolume2ProtocolGuid) → Find all FVs
3. For each FV:
   a. FV->ReadSection(gDTbtDxeModuleGuid, type=25) → Read raw ACPI section
   b. Search for ACPI table signature 0x7464735374625444 = "DtbStdt" (DTbt SSDT)
   c. Within table, search for marker 0x56445444 = "DTDV"
   d. Patch offset+9: Write gDTbtPolicy pointer address
   e. Patch offset+14: Write 42 (0x2A) — DTbt ACPI enable flag
   f. AcpiTable->InstallAcpiTable(patched table)
```

**This is where the GPE 0x6E handler gets enabled.** The patched SSDT contains the
`_L6E` method that handles Thunderbolt wake GPEs. Writing 42 at offset+14 sets the
enable flag that arms the GPE handler.

### Phase 3: PCI Force Power (DTbt_ProgramPciConfig, EndOfDxe callback)

```
For each DTbt port (up to 2 ports, 32-byte stride):
  If port enabled and port config present:
    1. Read PCI config Bus:Dev:Func from DTbt config structure
    2. Read secondary bus number from PCI offset 0x19
    3. If no valid secondary bus: temporarily assign bus (base+40)
    4. Read vendor/device ID from secondary bus device 0
    5. Check if Barlow Ridge (0x8086:0x5780 or 0x5783)
    6. If matched:
       a. WRITE PCI offset 0x54C = (gDTbtEnabled_Port1 << 8) | 0x63
          → Force Power ON + Command + Status bits
       b. POLL PCI offset 0x548 for completion (bit 0 set, 500ms timeout)
       c. WRITE PCI offset 0x54C = 0x00000000
          → Clear Force Power
       d. POLL PCI offset 0x548 for power-down (bit 0 clear, 500ms timeout)
    7. Restore original secondary/subordinate bus numbers
```

**PCI Register 0x54C (TBT_FORCE_POWER):**
- Bits [1:0]: Force Power Enable + Command
- Bits [15:8]: Controller slot/index
- Value `0x63` = Force Power ON with command active

### Phase 4: PCI Bus Enumeration (DTbt_ReadyToBootCallback, ReadyToBoot callback)

```
For each DTbt port (up to 2 ports):
  If port enabled:
    1. Build PCI address from Bus:Dev:Func config
    2. Call DTbt_EnumeratePciBus() — recursive PCI bridge walk
       - Checks bridge class code (0x06)
       - Reads secondary/subordinate bus range
       - Recursively enumerates downstream devices
       - Calls DTbt_FindPciExpressCapability() for PCIe capability discovery
       - Clears Master Abort bits along the path
```

---

## GPE 0x6E Root Cause Chain (Complete)

```
FIRMWARE (DTbtDxe.efi):
  DTbt_PatchAcpiSsdt()
    → Patches DTbt SSDT with policy data
    → Writes enable flag (42) at DTDV marker offset+14
    → Installs patched SSDT via ACPI Table Protocol
    → This activates the _L6E GPE handler in ACPI namespace

  DTbt_ProgramPciConfig()
    → Writes 0x63 to TBT PCI register 0x54C (Force Power)
    → Powers up Barlow Ridge Thunderbolt controller
    → Controller drives GPIO pin P1WG (Thunderbolt wake)

HARDWARE:
  Barlow Ridge TBT controller (8086:5780) drives P1WG GPIO
  PCH GPIO controller routes P1WG → GPE 0x6E (level-triggered)

ACPI (DSDT + SSDT27):
  _L6E handler fires when GPE 0x6E asserts
  PL6E() in SSDT27 checks P1WG GPIO
  Sends Device Wake (0x02) to PC02.RP21 (TBT root port)
  Checks GPU GPIOs: 0x160016 (HDMI), 0x160017 (TBT SNK0), 0x160015 (TBT SNK1)
  
  BUG: GPIO source not properly acknowledged before clear
  Level-triggered GPE re-fires immediately → ~400 fires/sec
  CPU pegged at 88°C processing ACPI interrupt storm

MITIGATION:
  acpi_mask_gpe=0x6E in GRUB → kernel masks GPE → temp drops to 64°C
  Side effect: Thunderbolt hotplug wake disabled (controller still works)
```

---

## Files

| File | Description |
|------|-------------|
| `/tmp/bios_analysis/thunderbolt/DTbtDxe.efi` | Original binary (loaded in IDA) |
| `/tmp/bios_analysis/thunderbolt/DTbtDxe_analysis.md` | This analysis report |
| `/tmp/DTbtDxe_decompiled.c` | Full decompiled C pseudocode (774 lines) |
| `/tmp/bios_analysis/acpi/DSDT.dsl` | Decompiled DSDT (102K lines, _L6E at 43853) |
| `/tmp/ssdt27.dsl` | Decompiled SSDT27 (PL6E handler at line 1036) |

---

*Analysis performed 2026-02-10 by IDA Pro 9.2 MCP integration.*
