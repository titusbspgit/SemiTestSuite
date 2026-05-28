#include "pcie_dbi_dsp_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register map defines for this test case (deterministic placeholders) */
#define REG_CMD_STATUS ((volatile uint32_t*)0x40000000u)
#define REG_CFG_SETUP  ((volatile uint32_t*)0x40000100u)
#define REG_OB_SIZE    ((volatile uint32_t*)0x40000200u)
#define REG_IRQ_EOI    ((volatile uint32_t*)0x40000300u)

static inline void write_reg(volatile uint32_t* addr, uint32_t val){ *addr = val; }
static inline uint32_t read_reg(volatile uint32_t* addr){ return *addr; }

void pcie_dbi_dsp_reg_wr_rd_test(void)
{
    printf("[TRACE] Start pcie_dbi_dsp_reg_wr_rd_test\n");
    printf("[TRACE] SS/Module: PCIE DBI DSP Registers, Feature: dbi_dsp_reg_wr_rd\n");
    printf("[TRACE] Description: Verify default values and basic read/write for DBI DSP registers.\n");

    // Step 1: Enable clocks (placeholder ops)
    printf("[TRACE] Step 1: Enable clocks\n");

    enum { NUM_REGS = 4 };
    volatile uint32_t* regs[NUM_REGS] = { REG_CMD_STATUS, REG_CFG_SETUP, REG_OB_SIZE, REG_IRQ_EOI };

    // Defaults hardcoded to 0 per plan Gap Analysis
    const uint32_t default_value_array[NUM_REGS] = { 0, 0, 0, 0 };

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
            printf("DSP_RST_FAIL idx=%u exp=0x%08X got=0x%08X\n", i, default_value_array[i], rd);
        }
    }

    // Step 3: Patterned writes
    printf("[TRACE] Step 3: Patterned writes and readback\n");
    const uint32_t patterns[] = { 0xAAAAAAAAu, 0x55555555u, 0x00000000u };
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
                printf("DSP_WR_FAIL idx=%u exp=0x%08X got=0x%08X\n", i, pat, rd);
            }
        }
    }

    // Step 4: Final validation
    printf("[TRACE] Step 4: Final validation: def_fail_cnt=%u, wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
    if ((def_fail_cnt != 0u) || (wr_fail_cnt != 0u))
    {
        printf("[RESULT] pcie_dbi_dsp_reg_wr_rd_test: FAIL\n");
        while(1) { }
    }

    printf("[RESULT] pcie_dbi_dsp_reg_wr_rd_test: PASS\n");
}
