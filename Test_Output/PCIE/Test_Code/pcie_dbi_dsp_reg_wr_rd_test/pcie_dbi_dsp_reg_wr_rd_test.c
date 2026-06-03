#include "pcie_dbi_dsp_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */
/* RAG partial data used: CMD_STATUS typically at offset 0x004 in DBI space.
 * CFG_SETUP/OB_SIZE/IRQ_EOI are device-specific; placeholders provided.
 */
#define PCIE_DBI_DSP_BASE             0x0  /* TODO: Replace with actual base */

#define CMD_STATUS_OFFSET             0x004 /* from RAG (typical) */
#define CFG_SETUP_OFFSET              0x000 /* TODO: Replace with actual offset */
#define OB_SIZE_OFFSET                0x000 /* TODO: Replace with actual offset */
#define IRQ_EOI_OFFSET                0x000 /* TODO: Replace with actual offset */

#define BASE_ADDR_CMD_STATUS          (PCIE_DBI_DSP_BASE + CMD_STATUS_OFFSET)
#define BASE_ADDR_CFG_SETUP           (PCIE_DBI_DSP_BASE + CFG_SETUP_OFFSET)
#define BASE_ADDR_OB_SIZE             (PCIE_DBI_DSP_BASE + OB_SIZE_OFFSET)
#define BASE_ADDR_IRQ_EOI             (PCIE_DBI_DSP_BASE + IRQ_EOI_OFFSET)

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
void pcie_dbi_dsp_reg_wr_rd_test(void)
{
    uint32_t val;
    unsigned i, p;
    unsigned def_fail_cnt = 0, wr_fail_cnt = 0;

    printf("Running %s\n", "pcie_dbi_dsp_reg_wr_rd_test");

    /* RAG data not complete, using placeholder offsets for some registers. */
    static const uint32_t addr_array[] = {
        BASE_ADDR_CMD_STATUS,
        BASE_ADDR_CFG_SETUP,
        BASE_ADDR_OB_SIZE,
        BASE_ADDR_IRQ_EOI
    };

    /* Defaults per Excel: hardcoded to 0 */
    static const uint32_t default_value_array[] = {
        0x00000000, /* CMD_STATUS */
        0x00000000, /* CFG_SETUP */
        0x00000000, /* OB_SIZE */
        0x00000000  /* IRQ_EOI */
    };

    /* No RO masking (Excel remark). Write to all registers. */
    static const uint32_t patterns[] = { 0xAAAAAAAAu, 0x55555555u, 0x00000000u };

    /* Step 1: Default check */
    for (i = 0; i < (sizeof(addr_array)/sizeof(addr_array[0])); ++i)
    {
        uint32_t exp = default_value_array[i];
        val = read_reg(addr_array[i]);
        if (val != exp)
        {
            ++def_fail_cnt;
            printf("DSP_RST_FAIL: idx=%u addr=0x%08X exp=0x%08X act=0x%08X\n", i, addr_array[i], exp, val);
        }
    }

    /* Step 2-4: Write patterns to all and verify */
    for (p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p)
    {
        for (i = 0; i < (sizeof(addr_array)/sizeof(addr_array[0])); ++i)
        {
            write_reg(addr_array[i], patterns[p]);
            val = read_reg(addr_array[i]);
            if (val != patterns[p])
            {
                ++wr_fail_cnt;
                printf("DSP_WR_FAIL: idx=%u pat=0x%08X addr=0x%08X exp=0x%08X act=0x%08X\n", i, patterns[p], addr_array[i], patterns[p], val);
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
