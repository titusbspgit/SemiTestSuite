#include "pcie_dbi_dsp_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */
// RAG data not available, using placeholder for DBI/DSP Application registers.
// TODO: Replace with actual base and offsets per device TRM.
#define BASE_ADDR_DBI_DSP               0x0  // TODO: Update
#define CMD_STATUS_OFFSET               0x000 // TODO: Update
#define CFG_SETUP_OFFSET                0x004 // TODO: Update
#define OB_SIZE_OFFSET                  0x008 // TODO: Update
#define IRQ_EOI_OFFSET                  0x00C // TODO: Update

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
void pcie_dbi_dsp_reg_wr_rd_test(void)
{
    printf("Running %s\n", "pcie_dbi_dsp_reg_wr_rd_test");

    const uint32_t offsets[] = {
        CMD_STATUS_OFFSET,
        CFG_SETUP_OFFSET,
        OB_SIZE_OFFSET,
        IRQ_EOI_OFFSET
    };

    const uint32_t default_value_array[] = {
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000
    };

    const uint32_t patterns[] = {0xAAAAAAAAu, 0x55555555u, 0x00000000u};

    unsigned def_fail_cnt = 0;
    unsigned wr_fail_cnt = 0;

    // Step 1: Read each DSP register and compare to defaults (Excel)
    for (unsigned i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i)
    {
        uint32_t addr = BASE_ADDR_DBI_DSP + offsets[i];
        uint32_t val = read_reg(addr);
        if (val != default_value_array[i])
        {
            ++def_fail_cnt;
            printf("DSP_RST_FAIL: index=%u addr=0x%08X read=0x%08X exp=0x%08X\n",
                   i, addr, val, default_value_array[i]);
        }
    }

    // Step 2/3: Write patterns then read back and compare (no masks per Excel)
    for (unsigned p = 0; p < sizeof(patterns)/sizeof(patterns[0]); ++p)
    {
        uint32_t pat = patterns[p];
        // Write to all
        for (unsigned i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i)
        {
            uint32_t addr = BASE_ADDR_DBI_DSP + offsets[i];
            write_reg(addr, pat);
        }
        // Readback/validate
        for (unsigned i = 0; i < sizeof(offsets)/sizeof(offsets[0]); ++i)
        {
            uint32_t addr = BASE_ADDR_DBI_DSP + offsets[i];
            uint32_t val = read_reg(addr);
            if (val != pat)
            {
                ++wr_fail_cnt;
                printf("DSP_WR_FAIL: index=%u addr=0x%08X read=0x%08X exp=0x%08X\n",
                       i, addr, val, pat);
            }
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
