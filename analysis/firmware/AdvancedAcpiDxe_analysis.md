# AdvancedAcpiDxe.efi — Complete Reverse Engineering Analysis

**Binary:** `/tmp/bios_analysis/acpi/AdvancedAcpiDxe.efi`
**Size:** 70,688 bytes (70KB)
**Functions:** 91 total — all decompiled and renamed
**Platform:** Lenovo ThinkPad P16 Gen 3 (Arrow Lake-S), Phoenix SecureCore BIOS N4FET30W (1.11)
**Analysis Date:** 2026-02-10
**Tool:** IDA Pro 9.2 via MCP Server (localhost:8080)

---

## Executive Summary

AdvancedAcpiDxe is the **master ACPI table construction engine** for the entire platform. It is responsible for:

1. **Allocating and populating the Global NVS (GNVS) area** — a 3499-byte shared memory region that ACPI methods read at runtime to access BIOS configuration
2. **Loading and installing SSDT tables** from the firmware volume
3. **Building dynamic AML devices** for Serial I/O (I2C, SPI, UART) peripherals
4. **Constructing the MADT/APIC processor table**
5. **Configuring PCIe root port NVS data** for runtime ACPI hotplug

### Relevance to GPE 0x6E Investigation

AdvancedAcpiDxe is the **source of the DTbt NVS configuration** that enables the GPE 0x6E interrupt storm:

- **NVS offset 831** = Discrete Thunderbolt Enable flag (defaults to `1`)
- **Setup variable offset 0x76D** (1901 decimal) = BIOS setup option that controls NVS+831
- When `Setup[0x76D] == 1`, NVS offsets 832-844 are populated with DTbt power management config
- DTbtDxe reads this NVS area and enables the Thunderbolt force-power mechanism
- This triggers the GPIO → GPE 0x6E → interrupt storm chain

**PCHA** (the `_L6E` handler gate at GNVS offset 4692) is **NOT** set by AdvancedAcpiDxe — it only fills the first 3499 bytes. PCHA must be set by PchInitDxe or SiInitDxe.

---

## Architecture Overview

```
_ModuleEntryPoint (0x474)
  ├── Stores gBS, gRT, gST pointers
  ├── Acpi_GetHobList() — caches HOB list pointer
  ├── Acpi_FindConfigTable() — finds config tables by GUID
  └── Acpi_MainNvsBuilder (0x2770) — THE MONSTER (316 vars, 105K pseudocode)
        ├── Reads 7 UEFI variables: Setup, SaSetup, MeSetup, CpuSetup, PchSetup, VtioCfg, DptfConfig
        ├── Locates ACPI Table protocol (gEfiAcpiTableProtocolGuid)
        ├── Calls Acpi_AllocInitNvsArea() — allocates 3499-byte GNVS, sets defaults
        ├── Populates ~1364 NVS fields from setup variables
        ├── Calls Acpi_BuildSerialIoSsdt() — dynamic Serial IO SSDT construction
        ├── Calls Acpi_InstallAllSsdts() x2 — installs SSDTs from FV
        ├── Patches existing ACPI tables via Acpi_FindPatchAcpiTbl()
        ├── Calls Acpi_EndOfDxeCallback() — registers EndOfDxe event
        │     ├── Acpi_LoadSsdtFromFV() — loads additional SSDTs from FV "ssdt.aml" files
        │     └── Acpi_LoadPerfTuneSsdt() — loads Intel PerfTune SSDT
        ├── Madt_BuildProcTable() — MADT/APIC processor enumeration
        ├── Acpi_ReadPcieRpNvsCfg() — PCIe root port config for NVS
        └── Acpi_S3MemoryVarHandler() — S3 resume memory save/restore
```

---

## GUID Identification (27 GUIDs)

### Standard UEFI GUIDs (4)

| Address | GUID | Name | Source |
|---------|------|------|--------|
| 0xE038 | FFE06BDD-6107-46A6-7BB2-5A9C7EC5275C | gEfiAcpiTableProtocolGuid | MdePkg |
| 0xE048 | EB97088E-CFDF-49C6-BE4B-D906A5B20E86 | gEfiAcpiSdtProtocolGuid | MdePkg |
| 0xE058 | 7739F24C-93D7-11D4-9A3A-0090273FC14D | gEfiHobListGuid | MdePkg |
| 0xDF28 | 2F707EBB-4A1A-11D4-9A38-0090273FC14D | gEfiAcpi20TableGuid | MdePkg |

### Intel Setup Variable GUIDs (6)

| Address | GUID | Variable Name | Size |
|---------|------|---------------|------|
| 0xDFF8 | 74B90584-08B1-9A61-43D6-87ECA4EBB54B | "Setup" | 3316 bytes |
| 0xE0C8 | 6440C38D-8472-8423-88E2-C5728377A143 | "SaSetup" | 932 bytes |
| 0xE108 | A04D2E95-8E39-B92B-B866-A3F5F4C15F4A | "MeSetup" | 243 bytes |
| 0xDFA8 | A5D590A2-3171-F70E-FF97-8FB0E8E69341 | "CpuSetup" | 1704 bytes |
| 0xE0B0 | 69E8FAB9-88D2-4FC6-F1B7-7045E8AD4349 | "PchSetup" | 6282 bytes |
| 0xE148 | 6E9A95BD-094F-C325-FBA3-18B38CC9F443 | "VtioCfg" | 340 bytes |

### Intel Platform Protocol GUIDs (9)

| Address | GUID | Identified As |
|---------|------|---------------|
| 0xDF08 | 3FDDA605-A76E-4F46-AD29-12F4531B3D08 | PchInfo protocol (GPIO/PCH services) |
| 0xDF18 | 72D1FFF7-A42A-4219-B995-5A67536EA42A | Unknown platform protocol |
| 0xDF38 | EA31AB0A-17F5-48E6-A21D-22D277D58682 | Unknown Intel DXE protocol |
| 0xDF48 | B6043ADA-F129-482C-82D9-64E5B02A985D | DptfConfig variable GUID |
| 0xDF58 | C148865B-4670-4A2C-A3ED-889A4FB055E5 | Device enumeration protocol |
| 0xDF68 | 0C02AA99-83FD-4FB5-9BC6-EEC23304CAAC | PerfTune SSDT FV file GUID |
| 0xDF80 | 84AF9E95-109C-8878-C1F0-5CAC82A68F47 | Unknown HOB GUID |
| 0xDF90 | 85F2A88A-1B09-852A-0640-B3115BD80A4D | Unknown HOB GUID |
| 0xDFB8 | 9E5E97A9-0A9B-32DB-926E-57093F6DD211 | SMBIOS table GUID |

### Firmware Volume File GUIDs (4)

| Address | GUID | FFS File |
|---------|------|----------|
| 0xE338 | 7E374E25-8E01-4FEE-87F2-390C23C606CD | AcpiTables (main SSDT collection) |
| 0xE320 | 6B5C8FE5-70DD-4E17-BFF4-D21C26586EB3 | Secondary SSDT collection |
| 0xDFE8 | 3EB2EC9E-503F-9A02-B673-0E22DB6B1344 | Firmware Volume 2 search GUID |
| 0xE028 | 964E5B22-6459-11D2-8E39-00A0C969723B | Simple File System protocol |

### Other GUIDs (4)

| Address | GUID | Purpose |
|---------|------|---------|
| 0xDFC8 | A000398E-69C9-3B72-F2BE-401188071143 | Unknown |
| 0xE000 | 3E3FE5A1-B236-A90D-F6F0-A3134A26F03E | Unknown |
| 0xE010 | C5DEE0F2-3412-342F-56E5-7AC7A340C041 | Unknown |
| 0xE080 | 8448348B-DFB5-59B3-55F0-AB27B8B1264C | Unknown |

---

## Function Map (91 Functions)

### Core ACPI Engine (13 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0x474 | `_ModuleEntryPoint` | DXE entry — stores gBS/gRT/gST, calls main builder |
| 0x2770 | `Acpi_MainNvsBuilder` | **THE MONSTER** — 316 vars, reads all setup vars, populates 3499-byte GNVS |
| 0xBD18 | `Acpi_AllocInitNvsArea` | Allocates GNVS, zeros 3499 bytes, sets hardware defaults |
| 0x2258 | `Acpi_EndOfDxeCallback` | EndOfDxe event — loads remaining SSDTs, installs CPU table patches |
| 0xBC34 | `Acpi_LocateAcpiProtos` | Locates AcpiSdt + AcpiTable protocols |
| 0xBC78 | `Acpi_FindPatchAcpiTbl` | Finds ACPI table by signature, patches fields in-place |
| 0xBAF8 | `Acpi_FindConfigTable` | Searches EFI Configuration Table for GUID |
| 0xBB58 | `Acpi_GetHobList` | Caches HOB list pointer |
| 0xBB88 | `Acpi_FindHobByGuid` | Walks HOB list for GUID Extension type |
| 0xB800 | `Acpi_AllocatePool` | gBS->AllocatePool wrapper |
| 0xB830 | `Acpi_AllocateZeroPool` | Allocate + zero-fill |
| 0xB85C | `Acpi_AllocateAndCopy` | Allocate + memcpy |
| 0xDDC8 | `Acpi_CallDevEnumProto` | Calls device enumeration protocol |

### SSDT Loading & Installation (7 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0x1ABC | `Acpi_InstallAllSsdts` | Iterates FV files by GUID, installs each SSDT via AcpiTable protocol |
| 0x1BC8 | `Acpi_GetSsdtFileSize` | Gets SSDT file size from FV |
| 0x1C80 | `Acpi_LoadSsdtFromFV` | Searches FV for "ssdt.aml" files, checks "ssdtfile" signature |
| 0x4D4 | `Acpi_LoadPerfTuneSsdt` | Loads Intel PerfTune SSDT from FV (GUID 0xDF68) |
| 0x650 | `Acpi_FindAndLoadSsdt` | Generic SSDT finder by GUID from FV |
| 0x7D0 | `Acpi_FindProtocolByGuid` | Helper: searches handles for protocol matching GUID |
| 0x8CC | `Acpi_InspectSsdtTable` | Inspects SSDT table header, checks OEM signature |

### Serial I/O & I2C Device Construction (10 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0xAC04 | `Acpi_CfgSerialIoDevs` | Configures Serial I/O devices (I2C, SPI, UART) from setup |
| 0xAF38 | `Acpi_BuildI2cDeviceAml` | Builds AML for I2C device (_HID, _CID, _UID, _CRS nodes) |
| 0xB2FC | `Acpi_BuildSerialIoSsdt` | Constructs complete Serial I/O SSDT with all device nodes |
| 0xC83C | `Acpi_BuildAmlScope` | Builds AML Scope/Device block with proper S3MemoryVariable |
| 0xB5F0 | `Pch_GetMaxSerialIo` | Returns max Serial I/O port count per SKU |
| 0xB648 | `Pch_GetSerialIoIdx` | Maps port number to Serial I/O controller index |
| 0xB6A4 | `Pch_GetI2cGpioPin` | Returns GPIO pin number for I2C interrupt |
| 0xB6F8 | `Pch_GetI2cGpioPad` | Returns GPIO pad for I2C |
| 0xB754 | `Pch_GetI2cGpioPinAlt` | Alternate GPIO pin mapping |
| 0xB7A8 | `Pch_GetI2cGpioPadAlt` | Alternate GPIO pad mapping |

### MADT/Processor Table (2 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0xA668 | `Madt_BuildProcTable` | Enumerates all processors, builds MADT entries with BSP detection |
| 0xA5E8 | `Madt_MarkCurrentProc` | Marks current processor in the table via MP protocol callback |

### PCIe Root Port Configuration (5 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0x1FF4 | `Acpi_ProbePcieRootPort` | Probes PCIe root port via protocol at GUID 0xDFD8 |
| 0x20B4 | `Acpi_FindPcieCapForNvs` | Finds PCIe Extended Capability for NVS (writes NVS+3303/3307) |
| 0xA88C | `Acpi_ReadPcieRpNvsCfg` | Reads PCIe RP PCI config registers 396/408/412/692 into NVS+3475 |
| 0xCD5C | `Pch_IsBehindPchRp` | Checks if device is behind PCH root port |
| 0xCDE0 | `Pch_HasDeviceOnBus` | Scans for devices on secondary bus |

### PCI Configuration Space Access (16 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0xD44C | `Pci_CfgReadByteRaw` | Raw byte read from ECAM (0xC0000000) |
| 0xD564 | `Pci_CfgWriteByteRaw` | Raw byte write to ECAM |
| 0xD6A8 | `Pci_CfgOrByte` | Read-modify-write: OR byte |
| 0xD6E4 | `Pci_CfgAndByte` | Read-modify-write: AND byte |
| 0xD720 | `Pci_CfgAndOrByte` | Read-modify-write: AND then OR byte |
| 0xD76C | `Pci_CfgReadWord` | 16-bit read from ECAM |
| 0xD86C | `Pci_CfgWriteWord` | 16-bit write |
| 0xD994 | `Pci_CfgOrWord` | OR word |
| 0xD9D4 | `Pci_CfgAndWord` | AND word |
| 0xDA14 | `Pci_CfgAndOrWord` | AND then OR word |
| 0xDA64 | `Pci_CfgReadDword` | 32-bit read from ECAM |
| 0xDB30 | `Pci_CfgWriteDword` | 32-bit write |
| 0xDC2C | `Pci_CfgOrDword` | OR dword |
| 0xDC68 | `Pci_CfgAndDword` | AND dword |
| 0xDCA4 | `Pci_CfgAndOrDword` | AND then OR dword |
| 0xDCF0 | `Pci_BuildVtable` | Populates PCI access vtable (12 function pointers) |

### PCH Platform Identification (7 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0xB524 | `Cpuid_GetFMS` | Returns CPU Family/Model/Stepping from CPUID leaf 1 |
| 0xB550 | `Pch_GetSocSku` | Reads MMIO 0xC00F8002 to identify SoC SKU (ARL-S/H/U/P) |
| 0xB594 | `Pch_IsArrowLake` | Returns true if ARL-S or ARL-P SKU |
| 0xB5BC | `Hob_GetSocCfgByte25` | Gets byte 25 from SoC config HOB |
| 0xD15C | `Pch_GetSocSegment` | Gets PCI segment from HOB |
| 0xD18C | `Pch_GetPcieRpGpioPin` | Returns GPIO pin for PCIe root port hotplug |
| 0xD1BC | `Pch_GetPcieRpGpioPad` | Returns GPIO pad for PCIe root port |

### CPU Identification (5 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0xC6B4 | `Cpuid_CheckFeatureFlag` | Checks CPUID feature flag (vendor string "GenuineIntel") |
| 0xC76C | `Cpu_ReadApicBase` | Reads MSR 0x1B (APIC base), handles x2APIC mode |
| 0xC7D8 | `Cpu_IsHybridArch` | Checks if hybrid (P+E core) architecture |
| 0xC814 | `Cpu_GetApicMode` | Returns 1 (xAPIC) or 2 (x2APIC) |
| 0xCE44 | `Pch_GetPcieRpAddress` | Computes PCIe root port ECAM address |

### PS/2 Keyboard/Mouse (2 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0xAA18 | `Kbc_WaitInputBufEmpty` | Polls KBC port 0x64 for input buffer empty (1000 iterations) |
| 0xAA74 | `Kbc_SendAuxCommand` | Sends command 0xF4 to PS/2 aux device (trackpoint detect) |

### SMBIOS (4 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0xBBD0 | `Acpi_LocateSmbiosProto` | Locates SMBIOS protocol, maps "INTC1082"/"INTC1084" strings |
| 0xCA54 | `Smbios_ParseDimmInfo` | Parses SMBIOS Type 17 (Memory Device) for DIMM info |
| 0xCFD8 | `Smbios_ValidateTable` | Validates SMBIOS table structure |
| 0xD034 | `Smbios_GetTableSize` | Calculates total SMBIOS entry size |
| 0xD084 | `Smbios_CloneEntry` | Deep-copies a SMBIOS entry |

### S3 Resume (1 function)

| Address | Name | Purpose |
|---------|------|---------|
| 0x1E34 | `Acpi_S3MemoryVarHandler` | Saves/restores "S3MemoryVariable" for S3 resume TSEG config |

### Utility Functions (10 functions)

| Address | Name | Purpose |
|---------|------|---------|
| 0x260 | `efi_memcpy` | memcpy |
| 0x2A0 | `efi_zeromem` | Zero-fill memory |
| 0x2C0 | `efi_memcmp` | Memory compare |
| 0x2E0 | `efi_memset` | memset (dword fill) |
| 0xDECC | `efi_memset_byte` | memset (byte fill) |
| 0x340 | `cpuid_wrapper` | CPUID instruction wrapper |
| 0x370 | `msr_read_write` | MSR read/write wrapper |
| 0xB914 | `Util_HexCharToValue` | Hex character to integer |
| 0xB944 | `Util_ParseHexString` | Parse hex string to integer |
| 0xBA30 | `Util_WideStrLen` | Wide string length |
| 0xBA68 | `Util_WideStrCopy` | Wide string copy |
| 0x25C8 | `Acpi_ParseGuidString` | Parse "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX" GUID string |
| 0xAB2C | `Acpi_GetPsDeviceCount` | Gets number of PS/2 pointing devices from PCH info |
| 0xD1F0 | `Pci_CfgReadByte` | High-level PCI config byte read with range check |
| 0xC914 | `Pci_GetHeaderType` | Returns PCI header type (Bridge vs Endpoint) |
| 0xC9D0 | `Pci_FindExtCap` | Walks PCIe extended capability list for ID 11 (PTM) |
| 0xB89C | `Acpi_LocateGpioProto` | Locates GPIO protocol |
| 0xB8D8 | `Acpi_LocatePchInfoProto` | Locates PCH Info protocol |

---

## Thunderbolt / GPE 0x6E Analysis

### NVS Area Layout — Thunderbolt Section (NVS offsets 805–844)

| NVS Offset | Size | Default | Setup Offset | Field Name | Description |
|------------|------|---------|-------------|------------|-------------|
| 805 | BYTE | 1 | 0x753 | DTbt_TrOsup | Thunderbolt OS support enable |
| 806 | BYTE | 9 | 0x754 | DTbt_TrOsupCount | OS support retry count |
| 807 | DWORD | 4000 | 0x755 | DTbt_TrOsupDelay | OS support delay (ms) |
| 811 | DWORD | 30000 | 0x759 | DTbt_RtPollDelay | Runtime poll delay (ms) |
| 815-816 | 2×BYTE | 9 | 0x75D | DTbt_TrPortCount[0-1] | Port count per controller |
| 817 | WORD | 0 | 0x75F | DTbt_TrPcieSlot | PCIe slot number |
| 819 | WORD | 1200 | 0x761 | DTbt_SmDelay | SMI delay (ms) |
| 821 | DWORD | 30000 | 0x763 | DTbt_SmPollDelay | SMI poll delay (ms) |
| 825 | WORD | 1200 | 0x767 | DTbt_RpDelay0 | Root port delay 0 (ms) |
| 827 | WORD | 1200 | 0x769 | DTbt_RpDelay1 | Root port delay 1 (ms) |
| 829 | WORD | 1000 | 0x76B | DTbt_RpDelay2 | Root port delay 2 (ms) |
| **831** | **BYTE** | **1** | **0x76D** | **DTbt_Enabled** | **Discrete TBT enable — CONTROLS GPE 0x6E** |
| 832 | BYTE | 7 | 0x76E | DTbt_ForcePower | Force power mode (gated by 831==1) |
| 833 | WORD | 0xFFFF | 0x76F | DTbt_DeviceId | Expected TBT device ID |
| 835 | DWORD | 30000 | 0x771 | DTbt_WakeDelay | Wake delay (ms) |
| 839-842 | 4×BYTE | 50 | 0x775 | DTbt_Rtd3Delay[0-3] | RTD3 power-down delays |
| 843 | BYTE | 7 | 0x779 | DTbt_Controller | TBT controller type |
| 844 | WORD | 16720 | 0x77A | DTbt_RootPortNum | Root port B:D:F encoding |

### The Complete GPE 0x6E Enablement Chain

```
BIOS Setup Variable "Setup" at offset 0x76D
    │
    ▼
AdvancedAcpiDxe::Acpi_AllocInitNvsArea()
    │  Defaults NVS+831 = 1 (DTbt enabled by default)
    ▼
AdvancedAcpiDxe::Acpi_MainNvsBuilder()
    │  Copies Setup[0x76D] → NVS+831
    │  If NVS+831 == 1: populates NVS+832..844
    ▼
DTbtDxe::DTbt_DriverInit()
    │  Reads NVS+831 via gDTbtNvsAreaProtocolGuid
    │  If enabled: patches DTbt SSDT (signature "DtbStdt")
    ▼
DTbtDxe::DTbt_ProgramPciConfig()
    │  Writes PCI register 0x54C = 0x63 (Force Power ON)
    ▼
Barlow Ridge (8086:5780) powers up
    │  Asserts GPIO P1WG (Thunderbolt wake pin)
    ▼
PCH routes GPIO P1WG → GPE 0x6E (level-triggered)
    │
    ▼
SSDT27 _L6E handler fires
    │  Checks PCHA == 1 (set by PchInitDxe, NOT AdvancedAcpiDxe)
    │  Calls PL6E() which tries to clear GPIO sources
    │  BUG: GPIO source not properly acknowledged before clear
    ▼
GPE 0x6E re-fires immediately (~400/sec) = INTERRUPT STORM
```

### Key Finding: Setup[0x76D] Is the Kill Switch

If `Setup[0x76D]` were set to `0`:
- NVS+831 would be `0`
- DTbtDxe would NOT patch the SSDT
- DTbtDxe would NOT write PCI 0x54C (no force power)
- Barlow Ridge would not power up
- GPIO P1WG would not assert
- GPE 0x6E would never fire

**However**, this option is likely hidden in the BIOS setup menu and not exposed to the user. The Setup.efi module (955KB) would need to be analyzed to find if/how this option can be accessed.

### Current Mitigation

The kernel parameter `acpi_mask_gpe=0x6E` masks the GPE at the OS level, preventing the interrupt storm but leaving the firmware in its buggy state. A proper fix would require either:

1. **BIOS update from Lenovo** fixing the `_L6E` GPIO acknowledgment
2. **Custom SSDT overlay** replacing `_L6E` with a proper handler
3. **Modifying Setup[0x76D]** via UEFI shell `setup_var` tool (risky)

---

## UEFI Variable Map

| Variable | GUID | Base Address | Size | Key TBT Fields |
|----------|------|-------------|------|-----------------|
| Setup | 74B90584-... | 0xE660 | 3316 | [0x42]=SSDT load enable, [0x76D]=DTbt enable |
| SaSetup | 6440C38D-... | 0x306 area | 932 | System Agent / GPU config |
| MeSetup | A04D2E95-... | 0xE560 | 243 | Management Engine config |
| CpuSetup | A5D590A2-... | 0x307 area | 1704 | CPU power/thermal config |
| PchSetup | 69E8FAB9-... | 0xF360 | 6282 | PCH/GPIO/PCIe config |
| VtioCfg | 6E9A95BD-... | 0x299 area | 340 | Virtualization TIO config |
| DptfConfig | B6043ADA-... | 0x297 area | 38 | DPTF thermal config |

---

## Embedded Data Structures

### Embedded AML Templates

The binary contains pre-built AML bytecode templates at addresses 0xE150-0xE350:
- **I2C Device template** — `_STA`, `_HID "INT00000"`, `_CID`, `_UID`, `_CRS` with I2C Connection Descriptor
- **SSDT Header template** — OEM ID "Intel", OEM Table ID filled at runtime, INTL compiler ID
- **Scope `\_SB.PCI0.I2CX`** — I2C controller scope path with variable index

### String References

| Address | String | Used By |
|---------|--------|---------|
| 0xE2B8 | "SaSetup" | Acpi_MainNvsBuilder — SaSetup variable read |
| 0xE2C8 | "Rtd3" | Acpi_MainNvsBuilder — RTD3 power management SSDT name |
| 0xE2D0 | "xh_" | Acpi_MainNvsBuilder — xHCI SSDT prefix |
| 0xE2D8 | "Ther_Rvp" | Acpi_MainNvsBuilder — Thermal SSDT name |
| 0xE2E8 | "I2Pm_Rvp" | Acpi_MainNvsBuilder — I2C Power Management SSDT |
| 0xE2F8 | "I2Pm_194" | Acpi_MainNvsBuilder — I2C PM variant SSDT |
| 0xE308 | `\ssdt.aml` | Acpi_LoadSsdtFromFV — FV file path |
| 0xE318 | "S3MemoryVariable" | Acpi_S3MemoryVarHandler — S3 resume data |
| 0xE330 | "CpuSetup" | Acpi_MainNvsBuilder — CpuSetup variable read |
| 0xE400 | "Setup" | Acpi_MainNvsBuilder — main Setup variable read |
| 0xE408 | "MeSetup" | Acpi_MainNvsBuilder — MeSetup variable read |
| 0xE418 | "PchSetup" | Acpi_MainNvsBuilder — PchSetup variable read |
| 0xE438 | "DptfConfig" | Acpi_MainNvsBuilder — DPTF config read |
| 0xE450 | "PseudoG3StateCounter" | Power state counter variable |
| 0xE478 | "INTC1082" | Acpi_LocateSmbiosProto — ARL-S Serial IO ACPI ID |
| 0xE488 | "INTC1084" | Acpi_LocateSmbiosProto — ARL-P Serial IO ACPI ID |

---

## Comparison with DTbtDxe and ITbtDxe

| Feature | AdvancedAcpiDxe | DTbtDxe | ITbtDxe |
|---------|----------------|---------|---------|
| Size | 70KB (91 functions) | 5.7KB (22 functions) | 4.9KB (18 functions) |
| Role | Master NVS builder | Discrete TBT driver | Integrated TBT driver |
| NVS ownership | Allocates and populates GNVS (3499 bytes) | Reads NVS+831, owns 42-byte DTbt NVS | Reads TCSS HOBs, owns 20-byte ITbt NVS |
| SSDT handling | Loads from FV, installs via AcpiTable protocol | Patches existing DTbt SSDT in-place | Patches existing TCSS SSDT in-place |
| GPE 0x6E role | Sets NVS+831 (DTbt enable flag) | Writes PCI 0x54C (direct cause) | Not involved |
| Setup dependency | Reads 7 UEFI variables | Reads DTbt HOB (pre-populated by PEI) | Reads TCSS/ITbt HOBs |
| Protocol dependencies | AcpiTable, AcpiSdt, PchInfo, GPIO, SMBIOS | AcpiTable, FV2, DTbtNvsArea, DTbtPolicy | AcpiSdt, AcpiTable, TcssDma, TcssPolicy |
