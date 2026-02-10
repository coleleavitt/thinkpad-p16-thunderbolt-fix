/*
 * nhi_regdump - Dump Barlow Ridge NHI MMIO registers
 * 
 * Reads key NHI registers from BAR0 to understand interrupt state.
 * Must be loaded as root. Outputs to kernel log (dmesg).
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/io.h>

#define NHI_VID  0x8086
#define NHI_DID  0x5781  /* Barlow Ridge NHI */

/* NHI BAR0 registers */
#define REG_RING_NOTIFY_BASE          0x37800
#define REG_RING_INT_CLEAR            0x37808
#define REG_RING_INTERRUPT_BASE       0x38200
#define REG_RING_INT_MASK_CLEAR_BASE  0x38208
#define REG_INT_THROTTLING_RATE       0x38c00
#define REG_INT_VEC_ALLOC_BASE        0x38c40
#define REG_CAPS                      0x39640
#define REG_DMA_MISC                  0x39864
#define REG_RESET                     0x39898
#define REG_INMAIL_DATA               0x39900
#define REG_INMAIL_CMD                0x39904
#define REG_OUTMAIL_CMD               0x3990c
#define REG_FW_STS                    0x39944

/* PCI config VS_CAP registers */
#define VS_CAP_9   0xc8
#define VS_CAP_10  0xcc
#define VS_CAP_11  0xd0
#define VS_CAP_15  0xe0
#define VS_CAP_16  0xe4
#define VS_CAP_18  0xec
#define VS_CAP_19  0xf0
#define VS_CAP_22  0xfc

static int __init nhi_regdump_init(void)
{
    struct pci_dev *pdev = NULL;
    void __iomem *bar0;
    u32 val, caps, hop_count, notify_regs, int_regs;
    int i;
    resource_size_t bar0_start, bar0_len;

    pr_info("nhi_regdump: Searching for Barlow Ridge NHI (8086:5781)...\n");

    pdev = pci_get_device(NHI_VID, NHI_DID, NULL);
    if (!pdev) {
        pr_err("nhi_regdump: NHI device 8086:5781 not found\n");
        return -ENODEV;
    }

    pr_info("nhi_regdump: Found NHI at %s\n", pci_name(pdev));

    /* Read PCI config VS_CAP registers */
    pr_info("nhi_regdump: === PCI Config VS_CAP Registers ===\n");
    pci_read_config_dword(pdev, VS_CAP_9, &val);
    pr_info("nhi_regdump: VS_CAP_9  (0xC8) = 0x%08x  FW_READY=%d\n", val, !!(val & BIT(31)));
    pci_read_config_dword(pdev, VS_CAP_18, &val);
    pr_info("nhi_regdump: VS_CAP_18 (0xEC) = 0x%08x  TBT2PCIe_DONE=%d\n", val, !!(val & BIT(0)));
    pci_read_config_dword(pdev, VS_CAP_19, &val);
    pr_info("nhi_regdump: VS_CAP_19 (0xF0) = 0x%08x  PCIe2TBT_VALID=%d\n", val, !!(val & BIT(0)));
    pci_read_config_dword(pdev, VS_CAP_22, &val);
    pr_info("nhi_regdump: VS_CAP_22 (0xFC) = 0x%08x  FORCE_POWER=%d  DMA_DELAY=0x%02x\n",
            val, !!(val & BIT(1)), (val >> 24) & 0xff);

    /* Map BAR0 */
    bar0_start = pci_resource_start(pdev, 0);
    bar0_len = pci_resource_len(pdev, 0);
    if (!bar0_start || !bar0_len) {
        pr_err("nhi_regdump: BAR0 not configured\n");
        pci_dev_put(pdev);
        return -EIO;
    }
    pr_info("nhi_regdump: BAR0 at 0x%llx, size %llu (0x%llx)\n",
            (u64)bar0_start, (u64)bar0_len, (u64)bar0_len);

    bar0 = ioremap(bar0_start, bar0_len);
    if (!bar0) {
        pr_err("nhi_regdump: Failed to ioremap BAR0\n");
        pci_dev_put(pdev);
        return -ENOMEM;
    }

    /* Read REG_CAPS to get hop count */
    caps = ioread32(bar0 + REG_CAPS);
    hop_count = caps & 0x7ff;
    pr_info("nhi_regdump: === NHI BAR0 MMIO Registers ===\n");
    pr_info("nhi_regdump: REG_CAPS       (0x39640) = 0x%08x  hop_count=%u  version=0x%02x\n",
            caps, hop_count, (caps >> 16) & 0xff);

    /* Read key control registers */
    val = ioread32(bar0 + REG_DMA_MISC);
    pr_info("nhi_regdump: REG_DMA_MISC   (0x39864) = 0x%08x  INT_AUTO_CLEAR=%d  DISABLE_AUTO_CLEAR=%d\n",
            val, !!(val & BIT(2)), !!(val & BIT(17)));

    val = ioread32(bar0 + REG_RESET);
    pr_info("nhi_regdump: REG_RESET      (0x39898) = 0x%08x  HRR=%d\n", val, !!(val & BIT(0)));

    val = ioread32(bar0 + REG_INMAIL_DATA);
    pr_info("nhi_regdump: REG_INMAIL_DATA(0x39900) = 0x%08x\n", val);

    val = ioread32(bar0 + REG_INMAIL_CMD);
    pr_info("nhi_regdump: REG_INMAIL_CMD (0x39904) = 0x%08x  OP_REQUEST=%d  ERROR=%d\n",
            val, !!(val & BIT(31)), !!(val & BIT(30)));

    val = ioread32(bar0 + REG_OUTMAIL_CMD);
    pr_info("nhi_regdump: REG_OUTMAIL_CMD(0x3990C) = 0x%08x  OPMODE=%d\n",
            val, (val >> 8) & 0xf);

    val = ioread32(bar0 + REG_FW_STS);
    pr_info("nhi_regdump: REG_FW_STS     (0x39944) = 0x%08x  NVM_AUTH_DONE=%d  CIO_RESET_REQ=%d  ICM_EN_CPU=%d  ICM_EN=%d\n",
            val, !!(val & BIT(31)), !!(val & BIT(30)), !!(val & BIT(2)), !!(val & BIT(0)));

    val = ioread32(bar0 + REG_INT_THROTTLING_RATE);
    pr_info("nhi_regdump: REG_INT_THROT  (0x38C00) = 0x%08x\n", val);

    /* Read Ring Notify registers (interrupt status) */
    notify_regs = (31 + 3 * hop_count) / 32;
    pr_info("nhi_regdump: === Ring Notify (Interrupt Status) — %u regs ===\n", notify_regs);
    for (i = 0; i < (int)notify_regs && i < 8; i++) {
        val = ioread32(bar0 + REG_RING_NOTIFY_BASE + i * 4);
        pr_info("nhi_regdump: RING_NOTIFY[%d] (0x%05x) = 0x%08x\n",
                i, REG_RING_NOTIFY_BASE + i * 4, val);
    }

    /* Read Ring Interrupt Enable registers */
    int_regs = (31 + 2 * hop_count) / 32;
    pr_info("nhi_regdump: === Ring Interrupt Enable — %u regs ===\n", int_regs);
    for (i = 0; i < (int)int_regs && i < 8; i++) {
        val = ioread32(bar0 + REG_RING_INTERRUPT_BASE + i * 4);
        pr_info("nhi_regdump: RING_INT_EN[%d] (0x%05x) = 0x%08x\n",
                i, REG_RING_INTERRUPT_BASE + i * 4, val);
    }

    /* Read INT_VEC_ALLOC registers */
    pr_info("nhi_regdump: === Interrupt Vector Allocation ===\n");
    for (i = 0; i < 4; i++) {
        val = ioread32(bar0 + REG_INT_VEC_ALLOC_BASE + i * 4);
        pr_info("nhi_regdump: INT_VEC_ALLOC[%d] (0x%05x) = 0x%08x\n",
                i, REG_INT_VEC_ALLOC_BASE + i * 4, val);
    }

    /* Scan for any non-zero values in undocumented interrupt-related area */
    pr_info("nhi_regdump: === Scanning 0x39800-0x39A00 for non-zero regs ===\n");
    for (i = 0x39800; i < 0x39A00; i += 4) {
        val = ioread32(bar0 + i);
        if (val != 0 && val != 0xdeadbeef && val != 0xdeadbeaf) {
            pr_info("nhi_regdump: [0x%05x] = 0x%08x\n", i, val);
        }
    }

    /* Scan 0x39900-0x39A00 (mailbox / fw status area) */
    pr_info("nhi_regdump: === Scanning 0x39900-0x39A00 ===\n");
    for (i = 0x39900; i < 0x39A00; i += 4) {
        val = ioread32(bar0 + i);
        if (val != 0 && val != 0xdeadbeef && val != 0xdeadbeaf) {
            pr_info("nhi_regdump: [0x%05x] = 0x%08x\n", i, val);
        }
    }

    /* Also scan 0x39940-0x39980 around FW_STS */
    pr_info("nhi_regdump: === Area around FW_STS (0x39940-0x39980) ===\n");
    for (i = 0x39940; i <= 0x39980; i += 4) {
        val = ioread32(bar0 + i);
        pr_info("nhi_regdump: [0x%05x] = 0x%08x\n", i, val);
    }

    /* Check if there's something in the LC (Link Controller) register area
     * The LC registers are typically at router offset starting from adapter regs.
     * For USB4 routers, check around 0x3A000+ and 0x3B000+ */
    pr_info("nhi_regdump: === Scanning LC area 0x39A00-0x3A000 ===\n");
    for (i = 0x39A00; i < 0x3A000; i += 4) {
        val = ioread32(bar0 + i);
        if (val != 0 && val != 0xdeadbeef && val != 0xdeadbeaf && val != 0xffffffff) {
            pr_info("nhi_regdump: [0x%05x] = 0x%08x\n", i, val);
        }
    }

    iounmap(bar0);
    pci_dev_put(pdev);

    pr_info("nhi_regdump: Done.\n");
    return -EAGAIN; /* Fail to load on purpose — just dump and exit */
}

static void __exit nhi_regdump_exit(void)
{
    /* Nothing to clean up */
}

module_init(nhi_regdump_init);
module_exit(nhi_regdump_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cole Leavitt");
MODULE_DESCRIPTION("Barlow Ridge NHI register dump for GPE 0x6E analysis");
