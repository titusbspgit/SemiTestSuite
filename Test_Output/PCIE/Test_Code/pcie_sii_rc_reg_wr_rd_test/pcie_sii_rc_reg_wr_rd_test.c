#include "pcie_sii_rc_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */
/* RAG data used: SII RC inbound window 0 offsets relative to SII_RC_CFG_BASE. */
#define SII_RC_CFG_BASE               0x0  /* TODO: Replace with actual SII RC config base */

#define IB_BAR0_OFFSET                0x300
#define IB_START0_LO_OFFSET           0x304
#define IB_OFFSET0_OFFSET             0x30C

#define BASE_ADDR_IB_BAR0             (SII_RC_CFG_BASE + IB_BAR0_OFFSET)
#define BASE_ADDR_IB_START_LO         (SII_RC_CFG_BASE + IB_START0_LO_OFFSET)
#define BASE_ADDR_IB_OFFSET           (SII_RC_CFG_BASE + IB_OFFSET0_OFFSET)

/* Low-level access */
static inline void write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

static inline void finish(int code)
{
    printf("finish(%d)\n", code);
}

/* Test Function */
void pcie_sii_rc_reg_wr_rd_test(void)
{
    uint32_t val;
    unsigned i, p;
    unsigned def_fail_cnt = 0, wr_fail_cnt = 0;

    printf("Running %s\n", "pcie_sii_rc_reg_wr_rd_test");

    static const uint32_t addr_array[] = {
        BASE_ADDR_IB_BAR0,
        BASE_ADDR_IB_START_LO,
        BASE_ADDR_IB_OFFSET
    };

    /* Reset defaults are 0 per RAG for inbound window registers */
    static const uint32_t default_value_array[] = {
        0x00000000, 0x00000000, 0x00000000
    };

    static const uint32_t patterns[] = { 0xFFFFFFFFu, 0x00000000u };

    /* Step 1: Default check */
    for (i = 0; i < (sizeof(addr_array)/sizeof(addr_array[0])); ++i)
    {
        uint32_t exp = default_value_array[i];
        val = read_reg(addr_array[i]);
        if (val != exp)
        {
            ++def_fail_cnt;
            printf("SII_RST_FAIL: idx=%u addr=0x%08X exp=0x%08X act=0x%08X\n", i, addr_array[i], exp, val);
        }
    }

    /* Step 2-4: Write patterns and verify */
    for (p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p)
    {
        for (i = 0; i < (sizeof(addr_array)/sizeof(addr_array[0])); ++i)
        {
            write_reg(addr_array[i], patterns[p]);
            val = read_reg(addr_array[i]);
            if (val != patterns[p])
            {
                ++wr_fail_cnt;
                printf("SII_WR_FAIL: idx=%u pat=0x%08X addr=0x%08X exp=0x%08X act=0x%08X\n", i, patterns[p], addr_array[i], patterns[p], val);
            }
        }
    }

    if (def_fail_cnt == 0 && wr_fail_cnt == 0)
    {
        printf("Test Passed\n");
        finish(0);
    }
    else
    {
        printf("Test Failed\n");
        finish(1);
    }
}
