#include "pcie_dbi_usp_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */
// Partial RAG data used: GPRx are contiguous 32-bit registers with 4-byte stride.
// Base address is device-specific and not available; using placeholder.
// TODO: Replace BASE_ADDR_DBI_USP and verify offsets per device TRM.
#define BASE_ADDR_DBI_USP               0x0  // TODO: Update
#define GPR0_OFFSET                     0x000 // From RAG pattern (contiguous)
#define GPR1_OFFSET                     0x004 // From RAG pattern (contiguous)
#define GPR2_OFFSET                     0x008 // From RAG pattern (contiguous)

/* Low-level access */
static inline void write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

/* Test Function */
void pcie_dbi_usp_reg_wr_rd_test(void)
{
    printf("Running %s\n", "pcie_dbi_usp_reg_wr_rd_test");

    const uint32_t offsets[] = {
        GPR0_OFFSET,
        GPR1_OFFSET,
        GPR2_OFFSET
    };

    const uint32_t default_value_array[] = {
        0x00000000,
        0x00000000,
        0x00000000
    };

    const uint32_t pattern = 0xA5A5A5A5u; // Excel: single write pattern

    unsigned def_fail_cnt = 0;
    unsigned wr_fail_cnt = 0;

    // Step 1: Read GPR registers and compare to defaults
    for (unsigned i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i)
    {
        uint32_t addr = BASE_ADDR_DBI_USP + offsets[i];
        uint32_t val = read_reg(addr);
        if (val != default_value_array[i])
        {
            ++def_fail_cnt;
            printf("USP_RST_FAIL: index=%u addr=0x%08X read=0x%08X exp=0x%08X\n",
                   i, addr, val, default_value_array[i]);
        }
    }

    // Step 2: Write pattern to all GPR registers
    for (unsigned i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i)
    {
        uint32_t addr = BASE_ADDR_DBI_USP + offsets[i];
        write_reg(addr, pattern);
    }

    // Step 3: Read back and compare
    for (unsigned i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i)
    {
        uint32_t addr = BASE_ADDR_DBI_USP + offsets[i];
        uint32_t val = read_reg(addr);
        if (val != pattern)
        {
            ++wr_fail_cnt;
            printf("USP_WR_FAIL: index=%u addr=0x%08X read=0x%08X exp=0x%08X\n",
                   i, addr, val, pattern);
        }
    }

    if (def_fail_cnt == 0 && wr_fail_cnt == 0)
    {
        printf("Test Passed\n");
    }
    else
    {
        printf("Test Failed\n");
        while(1);
    }
}
