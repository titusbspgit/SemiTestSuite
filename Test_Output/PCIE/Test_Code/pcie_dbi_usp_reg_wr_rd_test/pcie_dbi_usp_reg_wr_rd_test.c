#include "pcie_dbi_usp_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */
/* RAG partial data used: GPR0/1/2 are 32-bit RW scratch, reset 0. Offsets/base are device-specific. */
#define PCIE_DBI_USP_BASE             0x0  /* TODO: Replace with actual base */

#define GPR0_OFFSET                   0x000 /* TODO: Replace with actual offset */
#define GPR1_OFFSET                   0x000 /* TODO: Replace with actual offset */
#define GPR2_OFFSET                   0x000 /* TODO: Replace with actual offset */

#define BASE_ADDR_GPR0                (PCIE_DBI_USP_BASE + GPR0_OFFSET)
#define BASE_ADDR_GPR1                (PCIE_DBI_USP_BASE + GPR1_OFFSET)
#define BASE_ADDR_GPR2                (PCIE_DBI_USP_BASE + GPR2_OFFSET)

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
void pcie_dbi_usp_reg_wr_rd_test(void)
{
    uint32_t val;
    unsigned i;
    unsigned def_fail_cnt = 0, wr_fail_cnt = 0;

    printf("Running %s\n", "pcie_dbi_usp_reg_wr_rd_test");

    static const uint32_t addr_array[] = {
        BASE_ADDR_GPR0,
        BASE_ADDR_GPR1,
        BASE_ADDR_GPR2
    };

    /* Defaults: per RAG these reset to 0 */
    static const uint32_t default_value_array[] = {
        0x00000000, 0x00000000, 0x00000000
    };

    /* Step 1: Read and compare to defaults */
    for (i = 0; i < (sizeof(addr_array)/sizeof(addr_array[0])); ++i)
    {
        uint32_t exp = default_value_array[i];
        val = read_reg(addr_array[i]);
        if (val != exp)
        {
            ++def_fail_cnt;
            printf("USP_RST_FAIL: idx=%u addr=0x%08X exp=0x%08X act=0x%08X\n", i, addr_array[i], exp, val);
        }
    }

    /* Step 2: Single write pattern */
    for (i = 0; i < (sizeof(addr_array)/sizeof(addr_array[0])); ++i)
    {
        write_reg(addr_array[i], 0xA5A5A5A5u);
    }

    /* Step 3: Readback and compare */
    for (i = 0; i < (sizeof(addr_array)/sizeof(addr_array[0])); ++i)
    {
        val = read_reg(addr_array[i]);
        if (val != 0xA5A5A5A5u)
        {
            ++wr_fail_cnt;
            printf("USP_WR_FAIL: idx=%u addr=0x%08X exp=0xA5A5A5A5 act=0x%08X\n", i, addr_array[i], val);
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
