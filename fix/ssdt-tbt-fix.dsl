DefinitionBlock ("ssdt-tbt-fix.aml", "SSDT", 2, "COLE", "TBTFIX", 0x00000001)
{
    /*
     * Fix for GPE 0x6E interrupt storm on ThinkPad P16 Gen 3.
     * 
     * Root cause: Barlow Ridge TBT4 controller (8086:5780) holds GPIO P1WG
     * asserted whenever powered. The DSDT's _L6E handler clears GPIO/GPE 
     * status but can't de-assert the level-triggered source, causing ~400
     * interrupts/sec (160K+ events).
     *
     * Fix: Disable GPE 0x6E enable bit early during ACPI init. Linux's
     * thunderbolt driver uses MSI-X exclusively; the ACPI GPE path is
     * unused on Linux.
     *
     * GPE 0x6E = GPE number 110 = bit 14 in GPE0_STS/EN block 3.
     * The enable register is at ACPI base (ABAS) + 0x7C.
     */
    External (\_SB.PC02.ABAS, IntObj)

    Scope (\_SB)
    {
        Device (TFIX)
        {
            Name (_HID, "TBTF0001")
            Name (_STA, 0x0F)

            Method (_INI, 0, Serialized)
            {
                Local0 = \_SB.PC02.ABAS
                If ((Local0 != Zero))
                {
                    OperationRegion (GPE3, SystemIO, (Local0 + 0x7C), 0x04)
                    Field (GPE3, DWordAcc, NoLock, Preserve)
                    {
                        EN3A, 32
                    }

                    Local1 = EN3A
                    EN3A = (Local1 & 0xFFFFBFFF)
                }
            }
        }
    }
}
