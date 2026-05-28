#include "pcie_dbi_usp_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register map defines for this test case (deterministic placeholders) */
#define REG_GPR0 ((volatile uint32_t*)0x40000000u)
#define REG_GPR1 ((volatile uint32_t*)0x40000100u)
#define REG_GPR2 ((volatile uint32_t*)0x40000200u)

static inline void write_reg(volatile uint32_t* addr, uint32_t val){ *addr = val; }
static inline uint32_t read_reg(volatile uint32_t* addr){ return *addr; }

void pcie_dbi_usp_reg_wr_rd_test(void)
{
    printf("[TRACE] Start pcie_dbi_usp_reg_wr_rd_test\n");
    printf("[TRACE] SS/Module: PCIE DBI USP Registers, Feature: dbi_usp_reg_wr_rd\n");
    printf("[TRACE] Description: Verify default values and read/write for DBI USP GPR registers.\n");

    // Step 1: Enable clocks (placeholder ops)
    printf("[TRACE] Step 1: Enable clocks\n");

    enum { NUM_REGS = 3 };
    volatile uint32_t* regs[NUM_REGS] = { REG_GPR0, REG_GPR1, REG_GPR2 };

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
            printf("USP_RST_FAIL idx=%u exp=0x%08X got=0x%08X\n", i, default_value_array[i], rd);
        }
    }

    // Step 3: Single write pattern per plan (0xA5A5A5A5)
    printf("[TRACE] Step 3: Write 0xA5A5A5A5 to each GPR and validate\n");
    const uint32_t pat = 0xA5A5A5A5u;
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
            printf("USP_WR_FAIL idx=%u exp=0x%08X got=0x%08X\n", i, pat, rd);
        }
    }

    // Step 4: Final validation
    printf("[TRACE] Step 4: Final validation: def_fail_cnt=%u, wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
    if ((def_fail_cnt != 0u) || (wr_fail_cnt != 0u))
    {
        printf("[RESULT] pcie_dbi_usp_reg_wr_rd_test: FAIL\n");
        while(1) { }
    }

    printf("[RESULT] pcie_dbi_usp_reg_wr_rd_test: PASS\n");
}
