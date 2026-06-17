#include "pcie_sii_rc_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>
#include "test_common.h"  // Provides read_reg, write_reg, finish

/* Low-level access fallbacks (not used if test_common.h provides APIs). */
static inline void ag_write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;  /* exact line required */
}
static inline uint32_t ag_read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;  /* exact line required */
}

/* Register Definitions (RAG-enriched typical PCIeSS inbound window 0) */
#define PCIE_IB_BASE       0x00000000  // TODO: Replace with Application/Inb. Region base

#define IB_BAR0_OFFSET     0x00000000  // R/W, bits [3:0] valid
#define IB_START_LO_OFFSET 0x00000008  // R/W, [31:20] used (1 MB aligned)
#define IB_OFFSET_OFFSET   0x00000010  // R/W, [31:20] used (1 MB aligned)

#define DEFAULT_IB_BAR0        0x00000000
#define DEFAULT_IB_START_LO    0x00000000
#define DEFAULT_IB_OFFSET      0x00000000

#define IB_BAR0_RW_MASK        0x0000000Fu
#define IB_START_LO_RW_MASK    0xFFF00000u
#define IB_OFFSET_RW_MASK      0xFFF00000u

static inline uint32_t ib_addr(uint32_t offset)
{
    return (PCIE_IB_BASE + offset);
}

void pcie_sii_rc_reg_wr_rd_test(void)
{
    printf("Running %s\n", "pcie_sii_rc_reg_wr_rd_test");

    const char *names[] = { "IB_BAR0", "IB_START_LO", "IB_OFFSET" };
    const uint32_t offsets[] = { IB_BAR0_OFFSET, IB_START_LO_OFFSET, IB_OFFSET_OFFSET };
    const uint32_t def_vals[] = { DEFAULT_IB_BAR0, DEFAULT_IB_START_LO, DEFAULT_IB_OFFSET };
    const uint32_t masks[] = { IB_BAR0_RW_MASK, IB_START_LO_RW_MASK, IB_OFFSET_RW_MASK };

    uint32_t def_fail_cnt = 0;
    uint32_t wr_fail_cnt  = 0;

    /* Step 1/2: Default read and compare */
    for (unsigned i = 0; i < 3; ++i)
    {
        uint32_t rv = read_reg(ib_addr(offsets[i])) & masks[i];
        uint32_t exp = def_vals[i] & masks[i];
        if (rv != exp)
        {
            printf("SII_RST_FAIL: %s exp=0x%08X got=0x%08X mask=0x%08X\n", names[i], exp, rv, masks[i]);
            def_fail_cnt++;
        }
    }

    /* Step 3/4/5: Write patterns then read back */
    const uint32_t patterns[] = { 0xFFFFFFFFu, 0x00000000u };

    for (unsigned p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p)
    {
        for (unsigned i = 0; i < 3; ++i)
        {
            uint32_t addr = ib_addr(offsets[i]);
            uint32_t mask = masks[i];
            uint32_t wv   = (def_vals[i] & ~mask) | (patterns[p] & mask);
            write_reg(addr, wv);
            uint32_t rv = read_reg(addr) & mask;
            uint32_t exp = wv & mask;
            if (rv != exp)
            {
                printf("SII_WR_FAIL: %s pat=0x%08X exp=0x%08X got=0x%08X mask=0x%08X\n",
                       names[i], patterns[p], exp, rv, mask);
                wr_fail_cnt++;
            }
        }
    }

    if (def_fail_cnt > 0 || wr_fail_cnt > 0)
    {
        printf("Failures: def=%u, wr=%u\n", def_fail_cnt, wr_fail_cnt);
        printf("Test Failed\n");
        while(1);
        finish(1);
        return;
    }

    printf("Test Passed\n");
    finish(0);
}
