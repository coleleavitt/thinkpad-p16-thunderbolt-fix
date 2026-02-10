#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>

static int __init read_p1wg_init(void)
{
    void *gnvs;
    u32 p1wg, p1pg, p1rg, p2wg, p2pg, p2rg, p3wg;
    u8 p1rn, p1ep, p1rp, p2rn, p2ep, p2rp;
    phys_addr_t gnvs_base = 0x53F15000;

    gnvs = memremap(gnvs_base, 0x1000, MEMREMAP_WB);
    if (!gnvs) {
        pr_err("read_p1wg: Failed to memremap GNVS at 0x%llx\n", (u64)gnvs_base);
        return -ENOMEM;
    }

    p1wg = *(u32 *)(gnvs + 0x990);
    p1rn = *(u8 *)(gnvs + 0x994);
    p1pg = *(u32 *)(gnvs + 0x995);
    p1ep = *(u8 *)(gnvs + 0x999);
    p1rg = *(u32 *)(gnvs + 0x99A);
    p1rp = *(u8 *)(gnvs + 0x99E);

    p2wg = *(u32 *)(gnvs + 0x99F);
    p2rn = *(u8 *)(gnvs + 0x9A3);
    p2pg = *(u32 *)(gnvs + 0x9A4);
    p2ep = *(u8 *)(gnvs + 0x9A8);
    p2rg = *(u32 *)(gnvs + 0x9A9);
    p2rp = *(u8 *)(gnvs + 0x9AD);

    p3wg = *(u32 *)(gnvs + 0x9AE);

    pr_info("read_p1wg: === TBT GPIO Pads from GNVS (base=0x%llx) ===\n", (u64)gnvs_base);
    pr_info("read_p1wg: P1WG=0x%08x  P1RN=%u  P1PG=0x%08x  P1EP=%u  P1RG=0x%08x  P1RP=%u\n",
            p1wg, p1rn, p1pg, p1ep, p1rg, p1rp);
    pr_info("read_p1wg: P2WG=0x%08x  P2RN=%u  P2PG=0x%08x  P2EP=%u  P2RG=0x%08x  P2RP=%u\n",
            p2wg, p2rn, p2pg, p2ep, p2rg, p2rp);
    pr_info("read_p1wg: P3WG=0x%08x\n", p3wg);

    pr_info("read_p1wg: P1WG decoded: bits[31:24]=0x%02x bits[23:16]=0x%02x bits[15:0]=0x%04x\n",
            (p1wg >> 24) & 0xff, (p1wg >> 16) & 0xff, p1wg & 0xffff);

    pr_info("read_p1wg: === Also reading DTbt fields nearby ===\n");
    pr_info("read_p1wg: GNVS+0x33F (DTbt Enable) = 0x%02x\n", *(u8 *)(gnvs + 0x33F));
    pr_info("read_p1wg: GNVS+0x972 (UDGF) = 0x%02x\n", *(u8 *)(gnvs + 0x972));
    pr_info("read_p1wg: GNVS+0x98B (WRTO) = 0x%02x\n", *(u8 *)(gnvs + 0x98B));
    pr_info("read_p1wg: GNVS+0x98C (PRST) = 0x%08x\n", *(u32 *)(gnvs + 0x98C));

    memunmap(gnvs);
    return -EAGAIN;
}

static void __exit read_p1wg_exit(void) {}

module_init(read_p1wg_init);
module_exit(read_p1wg_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Read P1WG via memremap for GPE 0x6E analysis");
