#include "pcie_sii_rc_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register map defines for this test case (deterministic placeholders) */
#define REG_IB_BAR0     ((volatile uint32_t*)0x40000000u)
#define REG_IB_START_LO ((volatile uint32_t*)0x40000100u)
#define REG_IB_OFFSET   ((volatile uint32_t*)0x40000200u)

static inline void write_reg(volatile uint32_t* addr, uint32_t val){ *addr = val; }
static inline uint32_t read_reg(volatile uint32_t* addr){ return *addr; }

void pcie_sii_rc_reg_wr_rd_test(void)
{
    printf("[TRACE] Start pcie_sii_rc_reg_wr_rd_test\n");
    printf("[TRACE] SS/Module: PCIE SII RC Registers, Feature: sii_rc_reg_wr_rd\n");
    printf("[TRACE] Description: Verify default values and read/write for SII Root Complex inbound address registers.\n");

    // Step 1: Enable clocks (placeholder ops)
    printf("[TRACE] Step 1: Enable clocks\n");

    enum { NUM_REGS = 3 };
    volatile uint32_t* regs[NUM_REGS] = { REG_IB_BAR0, REG_IB_START_LO, REG_IB_OFFSET };

    const uint32_t default_value_array[NUM_REGS] = { 0, 0, 0 };

    uint32_t def_fail_cnt = 0;
    uint32_t wr_fail_cnt  = 0;

    // Step 2: Read defaults
    printf("[TRACE] Step 2: Validate reset/default values\n");
    for (unsigned i = 0; i < NUM_REGS; ++i)
    {
        uint32_t rd = read_reg(regs[i]);
        if (rd != default_value_array[i])
        {
            ++def_fail_cnt;
            printf("SII_RC_RST_FAIL idx=%u exp=0x%08X got=0x%08X\n", i, default_value_array[i], rd);
        }
    }

    // Step 3: Two write patterns per plan
    printf("[TRACE] Step 3: Patterned writes and readback (0xFFFFFFFF, 0x00000000)\n");
    const uint32_t patterns[] = { 0xFFFFFFFFu, 0x00000000u };
    for (unsigned p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p)
    {
        uint32_t pat = patterns[p];
        printf("[TRACE]  Write pattern 0x%08X\n", pat);
        for (unsigned i = 0; i < NUM_REGS; ++i)
        {
            write_reg(regs[i], pat);
        }
        for (unsigned i = 0; i < NUM_REGS; ++i)
        {
            uint32_t rd = read_reg(regs[i]);
            if (rd != pat)
            {
                ++wr_fail_cnt;
                printf("SII_RC_WR_FAIL idx=%u exp=0x%08X got=0x%08X\n", i, pat, rd);
            }
        }
    }

    // Step 4: Final validation
    printf("[TRACE] Step 4: Final validation: def_fail_cnt=%u, wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
    if ((def_fail_cnt != 0u) || (wr_fail_cnt != 0u))
    {
        printf("[RESULT] pcie_sii_rc_reg_wr_rd_test: FAIL\n");
        while(1) { }
    }

    printf("[RESULT] pcie_sii_rc_reg_wr_rd_test: PASS\n");
}
