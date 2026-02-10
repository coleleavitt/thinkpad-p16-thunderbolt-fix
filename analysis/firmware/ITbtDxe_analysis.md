# ITbtDxe.efi — Complete Reverse Engineering Analysis

**Target:** ITbtDxe.efi (Integrated Thunderbolt DXE Driver)
**System:** Lenovo ThinkPad P16 Gen3 (21RQ002XUS), Arrow Lake-S
**BIOS:** N4FET30W (1.11), Phoenix SecureCore
**Binary:** PE32+ x86-64, 18 functions, entry at 0x360
**Tool:** IDA Pro 9.2 via MCP JSON-RPC, Hex-Rays Decompiler 9.3

---

## Executive Summary

ITbtDxe.efi is the **Integrated Thunderbolt** (TCSS — Type-C Sub-System) DXE driver,
the counterpart to DTbtDxe.efi which handles **Discrete Thunderbolt**. This driver
initializes the CPU-integrated Thunderbolt/USB4 controller present on Arrow Lake-S.

Unlike DTbtDxe which programs external Barlow Ridge PCI registers, ITbtDxe manages
the SoC-integrated TCSS ports. It patches the TCSS ACPI SSDT (signature `TcssSsdt`)
with ITbt configuration data and enumerates the integrated Thunderbolt PCI bus.

**This driver is NOT the root cause of GPE 0x6E.** The GPE 0x6E storm originates from
the Discrete Thunderbolt path (DTbtDxe + SSDT27 `_L6E`). However, ITbtDxe shares the
same ACPI table patching pattern and PCI bus enumeration code.

---

## Comparison: DTbtDxe vs ITbtDxe

| Aspect | DTbtDxe | ITbtDxe |
|--------|---------|---------|
| **Controller** | Discrete (Barlow Ridge 8086:5780) | Integrated (TCSS/CPU) |
| **Functions** | 22 | 18 |
| **Binary size** | 5.7K | 4.9K |
| **SSDT signature** | `DtbStdt` | `TcssSsdt` |
| **Data marker** | `DTDV` (0x56445444) | `ITBT` (0x42545449) |
| **PCI Force Power** | Yes (reg 0x54C) | No |
| **ACPI patching** | Yes (FirmwareVolume2 search) | Yes (ACPI SDT protocol) |
| **NVS area size** | 42 bytes (0x2A) | 20 bytes (0x14) |
| **Port count** | 2 | 4 |
| **Port config stride** | 32 bytes | 5 bytes |
| **GPE involvement** | Direct (enables _L6E via SSDT patch) | Indirect (separate SSDT) |

---

## GUID Table (14 GUIDs)

### Standard UEFI GUIDs (5 identified)

| Address  | Symbolic Name                      | GUID                                   |
|----------|------------------------------------|----------------------------------------|
| `0x1030` | `gEfiAcpiTableProtocolGuid`        | `FFE06BDD-6107-46A6-7BB2-5A9C7EC5275C` |
| `0x1040` | `gEfiAcpiSdtProtocolGuid`          | `EB97088E-CFDF-49C6-BE4B-D906A5B20E86` |
| `0x1050` | `gEfiHobListGuid`                  | `7739F24C-93D7-11D4-9A3A-0090273FC14D` |
| `0x1060` | `gEfiEventExitBootServicesGuid`    | `27ABF055-B1B8-4C26-8048-748F37BAA2DF` |
| `0x1080` | `gEfiEndOfDxeEventGroupGuid`       | `02CE967A-DD7E-4FFC-9EE7-810CF0470880` |

### Intel TCSS/ITbt Proprietary GUIDs (9, named by usage)

| Address  | Name (by usage analysis)           | GUID                                   | Role       |
|----------|-----------------------------------|----------------------------------------|------------|
| `0x0FC0` | `gTcssDmaProtocolGuid`            | `3C7BC880-41F8-4869-AEFC-870A3ED28299` | CONSUMES   |
| `0x0FD0` | `gTcssDmaInfoHobGuid`             | `992C52C8-BC01-4ECD-20BF-F957160E9EF7` | CONSUMES   |
| `0x0FE0` | `gITbtNvsAreaProtocolGuid`        | `DABF85BD-FBDC-4ED2-B10D-C908D08CEEE8` | **PRODUCES** |
| `0x0FF0` | `gITbtDisableBmeProtocolGuid`     | `89A9ADC3-9B7C-4B53-82BF-78726B914F9F` | PRODUCES   |
| `0x1000` | `gITbtConfigHobGuid`              | `455702CE-4ADB-45D9-8B27-F7B0D9798AE0` | CONSUMES   |
| `0x1010` | `gITbtPolicyHobGuid`              | `74A81EAA-033C-4783-BE2B-848574A697B7` | CONSUMES   |
| `0x1020` | `gTcssInfoHobGuid`                | `F1187E54-995F-49D9-ACEE-C534F45A18C7` | CONSUMES   |
| `0x1070` | `gTcssPolicyProtocolGuid`         | `B0563C42-28EA-40E6-9984-D5BFF8B04056` | CONSUMES   |
| `0x1090` | `gITbtHobSearchGuid`              | `196BF9E3-20D7-4B7B-89F9-31C27208C9B9` | Internal   |

**Note:** These Intel TCSS GUIDs are from the Arrow Lake-S silicon package, newer than
the ElkhartLake reference code. They are not publicly indexed in any GUID database.

---

## Function Map (18 Functions, Fully Decompiled)

### Core Driver Logic

| Address | Name                           | Purpose                                                           |
|---------|--------------------------------|-------------------------------------------------------------------|
| `0x360` | `_ModuleEntryPoint`            | Entry; saves gBS/gST, reads TCSS/ITbt HOBs, calls DriverInit     |
| `0x5A4` | `ITbt_DriverInit`              | **Main init** — scans TCSS PCI, allocates NVS, registers callbacks |
| `0x53C` | `ITbt_PatchAcpiSsdt`           | Patches TCSS SSDT with 'ITBT' and 'IACL' data fields              |
| `0x4D4` | `ITbt_EndOfDxeCallback`        | EndOfDxe — checks TCSS DMA VT-d, sets NVS flag                    |
| `0x3F8` | `ITbt_ReadyToBootCallback`     | ReadyToBoot — enumerates ITbt PCI bus for each enabled port        |

### ACPI Table Manipulation

| Address | Name                           | Purpose                                                           |
|---------|--------------------------------|-------------------------------------------------------------------|
| `0x9E8` | `ITbt_LocateAcpiProtocols`     | Locates gEfiAcpiSdtProtocolGuid + gEfiAcpiTableProtocolGuid       |
| `0xA2C` | `ITbt_FindAcpiTable`           | Enumerates ACPI tables via SDT protocol, matches by OEM table ID   |
| `0xAEC` | `ITbt_PatchAcpiTableField`     | Searches ACPI table for AML opcode pattern, patches field in-place |

### HOB / Memory Helpers

| Address | Name                    | Purpose                                                          |
|---------|-------------------------|------------------------------------------------------------------|
| `0x924` | `ITbt_GetHobData`       | Searches SystemTable->ConfigurationTable for gEfiHobListGuid     |
| `0x990` | `ITbt_FindHobByGuid`    | Walks HOB list for type=4 (GUID Extension) matching given GUID   |
| `0x8AC` | `ITbt_FindHobEntry`     | Searches within HOB data for matching GUID sub-entry              |

### PCI Bus Enumeration (identical pattern to DTbtDxe)

| Address | Name                               | Purpose                                                |
|---------|------------------------------------|---------------------------------------------------------|
| `0xCB8` | `ITbt_EnumeratePciBus`             | Recursive PCI bridge walk (same as DTbt_EnumeratePciBus)|
| `0xC04` | `ITbt_BuildPciAddress`             | Bus:Dev:Func → PCI ECAM address conversion              |
| `0xC44` | `ITbt_ClearPciMasterAbort`         | Clear PCI Status Master Abort bit                       |
| `0xEEC` | `ITbt_FindPciExpressCapability`    | Walk PCI capability list for PCIe cap (ID=0x10)         |

### Utility Functions

| Address | Name           | Purpose                                      |
|---------|----------------|----------------------------------------------|
| `0x280` | `efi_memset`   | Standard memset                              |
| `0x2A0` | `efi_memmove`  | Overlapping-safe memory copy                 |
| `0x340` | `efi_memcmp`   | Byte-by-byte comparison (returns difference) |

---

## Global Variables

| Address    | Name                   | Type    | Purpose                                        |
|------------|------------------------|---------|-------------------------------------------------|
| `0x10F8`   | `gBS`                  | PTR     | EFI_BOOT_SERVICES pointer                      |
| `0x10F0`   | `gST`                  | PTR     | EFI_SYSTEM_TABLE pointer                       |
| `0x1100`   | `gHobData`             | PTR     | Cached HOB list pointer                        |
| `0x1108`   | `gAcpiSdtProtocol`     | PTR     | EFI_ACPI_SDT_PROTOCOL instance                 |
| `0x1110`   | `gAcpiTableProtocol`   | PTR     | EFI_ACPI_TABLE_PROTOCOL instance               |
| `0x1118`   | `gITbtPortsConfigured` | BYTE    | Flag: ports populated from HOB (0 or 1)        |
| `0x1120`   | `gITbtPolicy`          | PTR     | ITbt NVS area — 20-byte policy structure        |
| `0x10B0`   | `gITbtPortConfig`      | BYTE[20]| Port config array: 4 ports × 5 bytes each      |

### ITbt Port Config Structure (5 bytes per port, 4 ports)

```
Offset 0: Port type (0 = not configured)
Offset 1: PCI Bus number
Offset 2: PCI Device number (5 bits)
Offset 3: Root port assignment (3 bits from HOB+44 DWORD, shifted by port*3)
Offset 4: Port enable flag (1 = enabled, from HOB+48+port_index)
```

---

## Driver Execution Flow

### Phase 1: Entry Point

```
1. Save gBS (SystemTable->BootServices) and gST (SystemTable)
2. ITbt_GetHobData() → Cache HOB list pointer
3. ITbt_FindHobByGuid(gTcssInfoHobGuid) → Get TCSS info
4. ITbt_FindHobByGuid(gITbtConfigHobGuid) → Get ITbt config
   - Check HOB+24: bit 0 must be set (TCSS enabled)
   - Read HOB+44: Root port assignment DWORD (3 bits per port)
   - Read HOB+48..51: Per-port enable flags
   - Populate gITbtPortConfig[0..3] (4 ports × 5 bytes)
   - Set gITbtPortsConfigured = 1
5. Call ITbt_DriverInit()
```

### Phase 2: Driver Init (ITbt_DriverInit)

```
1. Scan PCI ECAM for TCSS presence:
   - Check bus 0xD (0x68000), device offsets 0x2000 and 0x3000
   - Read vendor/device ID at each; if != 0xFFFF, TCSS present
2. If TCSS present:
   a. LocateProtocol(gTcssPolicyProtocolGuid) → Get TCSS policy
   b. ITbt_FindHobEntry(policy, gITbtHobSearchGuid) → Get ITbt sub-config
   c. ITbt_FindHobByGuid(gITbtConfigHobGuid) → Get ITbt HOB
   d. ITbt_FindHobByGuid(gITbtPolicyHobGuid) → Get ITbt policy HOB
   e. AllocatePool(20 bytes) → gITbtPolicy (NVS area)
   f. Populate NVS area from HOB data:
      - NVS+8: HOB+100 (TCSS DMA config)
      - NVS+9: HOB+32 (TCSS force power setting)
      - NVS+10: HOB+34 (TCSS connection manager type)
      - NVS+18: HOB+102 (TCSS RTD3 enable)
   g. If policy HOB+24 has per-port enable flags:
      - Set NVS+0 = 1 (ITbt enabled)
      - Copy port status array into NVS+4..7
      - Copy additional policy into NVS+13..16
   h. Check if config HOB+36 bit 31 set → NVS+12 = wake-on-USB flag
   i. If HOB+33 == 1 (EndOfDxe callback needed):
      - CreateEventEx(EndOfDxe, ITbt_EndOfDxeCallback)
   j. InstallProtocolInterface(gITbtNvsAreaProtocolGuid, gITbtPolicy)
   k. CreateEventEx(EndOfDxe, ITbt_PatchAcpiSsdt)
   l. LocateProtocol(gTcssPolicyProtocolGuid) again → check HOB+28 == 1
   m. If ITbt enabled:
      - CreateEventEx(ExitBootServices, ITbt_ReadyToBootCallback)
      - InstallProtocolInterface(gITbtDisableBmeProtocolGuid, sub_3F8)
```

### Phase 3: ACPI SSDT Patching (ITbt_PatchAcpiSsdt, EndOfDxe callback)

```
Unlike DTbtDxe which uses FirmwareVolume2 to find the raw SSDT,
ITbtDxe uses the EFI_ACPI_SDT_PROTOCOL to enumerate installed tables:

1. ITbt_LocateAcpiProtocols() → Get SDT + Table protocols
2. Build search signature: "TcssSsdt" (0x7464735373736354)
3. ITbt_FindAcpiTable("TcssSsdt") → Find TCSS SSDT by OEM table ID
4. ITbt_PatchAcpiTableField(table, 'ITBT', gITbtPolicy_ptr, 4)
   → Patch the ITbt NVS pointer into the SSDT
5. ITbt_PatchAcpiTableField(table, 'IACL', 20, 2)
   → Patch the NVS area size (20 bytes) into the SSDT
6. Recalculates ACPI checksum after each patch
```

### Phase 4: EndOfDxe Callback (ITbt_EndOfDxeCallback)

```
1. LocateProtocol(gTcssDmaProtocolGuid) → Get TCSS DMA protocol
2. Call protocol+64 (GetVtdState) → Check VT-d status
3. If VT-d disabled (return value == 0):
   a. FindHobByGuid(gTcssDmaInfoHobGuid) → Get DMA info
   b. Check HOB+52 bits [15:12] == 0x1000 (specific DMA controller)
   c. Set gITbtPolicy+17 = 1 (DMA controller without VT-d flag)
```

### Phase 5: ReadyToBoot (ITbt_ReadyToBootCallback)

```
For each of 4 ports (5-byte stride in gITbtPortConfig):
  If port index < 4 AND gITbtPortsConfigured:
    If port enable flag (offset+4) == 1:
      Build PCI B:D:F from port config
      Call ITbt_EnumeratePciBus() — recursive PCI bridge walk
  Else:
    Re-read ITbt config HOB, check HOB+48+port_index
    If enabled:
      Build PCI B:D:F from HOB root port assignment
      Call ITbt_EnumeratePciBus()
  
  CloseEvent(event) at end — deregisters callback
```

---

## Key Differences from DTbtDxe

1. **No PCI Force Power register writes** — TCSS integrated controllers don't need
   external force power; they're managed by the PCH directly.

2. **ACPI SDT Protocol instead of FirmwareVolume2** — ITbtDxe uses the cleaner
   `EFI_ACPI_SDT_PROTOCOL` API to find and patch tables, rather than raw FV search.

3. **4 ports vs 2 ports** — TCSS supports up to 4 USB4/Thunderbolt ports.

4. **No direct GPE involvement** — ITbtDxe patches the `TcssSsdt` which contains
   the integrated Thunderbolt ACPI methods, separate from the `_L6E` GPE handler
   in SSDT27 which is specific to the discrete Thunderbolt path.

5. **VT-d/DMA awareness** — ITbtDxe checks TCSS DMA controller VT-d state, which
   DTbtDxe does not (discrete TBT handles DMA differently).

---

## Files

| File | Description |
|------|-------------|
| `/tmp/bios_analysis/thunderbolt/ITbtDxe.efi` | Original binary (loaded in IDA) |
| `/tmp/bios_analysis/thunderbolt/ITbtDxe_analysis.md` | This analysis report |
| `/tmp/ITbtDxe_decompiled.c` | Full decompiled C pseudocode (697 lines) |

---

*Analysis performed 2026-02-10 by IDA Pro 9.2 MCP integration.*
