#!/bin/bash
# Build and install the SSDT overlay for GPE 0x6E fix
# Requires: iasl (Intel ACPI compiler), root access
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DSL="$SCRIPT_DIR/ssdt-tbt-fix.dsl"
AML="$SCRIPT_DIR/ssdt-tbt-fix.aml"

echo "=== Building SSDT overlay ==="
iasl -p "$SCRIPT_DIR/ssdt-tbt-fix" "$DSL"
echo "Compiled: $AML ($(stat -c%s "$AML") bytes)"

echo ""
echo "=== Creating initramfs CPIO ==="
TMPDIR=$(mktemp -d)
mkdir -p "$TMPDIR/kernel/firmware/acpi"
cp "$AML" "$TMPDIR/kernel/firmware/acpi/"
cd "$TMPDIR"
find . -type f | cpio -o -H newc > "$SCRIPT_DIR/acpi_override.cpio" 2>/dev/null
rm -rf "$TMPDIR"
echo "Created: $SCRIPT_DIR/acpi_override.cpio"

echo ""
echo "=== Installation ==="
echo "1. Copy CPIO to /boot:"
echo "   sudo cp $SCRIPT_DIR/acpi_override.cpio /boot/"
echo ""
echo "2. Add to /etc/default/grub:"
echo "   GRUB_EARLY_INITRD_LINUX_CUSTOM=\"acpi_override.cpio\""
echo ""
echo "3. Regenerate GRUB config:"
echo "   sudo grub-mkconfig -o /boot/grub/grub.cfg"
echo ""
echo "4. Also add kernel parameter (belt-and-suspenders):"
echo "   GRUB_CMDLINE_LINUX=\"... acpi_mask_gpe=0x6E\""
echo ""
echo "5. Reboot and verify:"
echo "   cat /sys/firmware/acpi/interrupts/gpe6E"
echo "   dmesg | grep -i 'ACPI.*SSDT\|TFIX\|Table Upgrade'"
