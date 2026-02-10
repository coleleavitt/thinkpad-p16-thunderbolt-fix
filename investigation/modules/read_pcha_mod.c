#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>

static int __init read_pcha_init(void)
{
    void *pnvb;
    u8 pcha, cppe, arlm;
    u16 pchs, pchg, pstp;
    
    /* PNVB = 0x53F2C018, size 0x4AF - this is in system RAM, use memremap */
    pnvb = memremap(0x53F2C018, 0x4AF, MEMREMAP_WB);
    if (!pnvb) {
        pr_err("read_pcha: memremap failed\n");
        return -ENOMEM;
    }
    
    pchs = *(u16 *)(pnvb + 0);
    pchg = *(u16 *)(pnvb + 2);
    pstp = *(u16 *)(pnvb + 4);
    cppe = *(u8 *)(pnvb + 1192);
    pcha = *(u8 *)(pnvb + 1193);
    arlm = *(u8 *)(pnvb + 1194);
    
    pr_info("read_pcha: PNVB=0x53F2C018\n");
    pr_info("read_pcha: PCHS=0x%04X PCHG=0x%04X PSTP=0x%04X\n", pchs, pchg, pstp);
    pr_info("read_pcha: CPPE=%u PCHA=%u ARLM=%u\n", cppe, pcha, arlm);
    
    if (pcha == 0)
        pr_info("read_pcha: PCHA=0 -> PCH ACPI DISABLED -> _L6E skips PL6E\n");
    else if (pcha == 1)
        pr_info("read_pcha: PCHA=1 -> PCH ACPI ENABLED -> _L6E calls PL6E -> GPE STORM!\n");
    else
        pr_info("read_pcha: PCHA=%u -> UNEXPECTED\n", pcha);
    
    memunmap(pnvb);
    return -ENODEV; /* unload immediately */
}

static void __exit read_pcha_exit(void) {}

module_init(read_pcha_init);
module_exit(read_pcha_exit);
MODULE_LICENSE("GPL");
