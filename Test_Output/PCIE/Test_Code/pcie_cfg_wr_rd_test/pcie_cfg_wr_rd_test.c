#include "pcie_cfg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register map defines for this test case (deterministic placeholders) */
#define REG_VENDOR_DEVICE_ID ((volatile uint32_t*)0x40000000u)
#define REG_STATUS_COMMAND   ((volatile uint32_t*)0x40000100u)
#define REG_CLASSCODE        ((volatile uint32_t*)0x40000200u)
#define REG_BAR0             ((volatile uint32_t*)0x40000300u)
#define REG_INT_PIN          ((volatile uint32_t*)0x40000400u)

static inline void write_reg(volatile uint32_t* addr, uint32_t val){ *addr = val; }
static inline uint32_t read_reg(volatile uint32_t* addr){ return *addr; }

void pcie_cfg_wr_rd_test(void)
{
    // Trace start and parameters
    printf("[TRACE] Start pcie_cfg_wr_rd_test\n");
    printf("[TRACE] SS/Module: PCIE Config Registers, Feature: cfg_wr_rd\n");
    printf("[TRACE] Description: Validate default reset values and masked read/write behavior for selected PCIe configuration registers.\n");

    // Step 1: Enable clocks (placeholder ops)
    printf("[TRACE] Step 1: Enable clocks\n");
    // TODO: Implement platform-specific clock enabling if applicable

    // Setup register list and masks per plan remarks
    enum { NUM_REGS = 5 };
    volatile uint32_t* regs[NUM_REGS] = {
        REG_VENDOR_DEVICE_ID,
        REG_STATUS_COMMAND,
        REG_CLASSCODE,
        REG_BAR0,
        REG_INT_PIN
    };

    // Default reset values are hardcoded to 0 per Gap Analysis in the plan
    const uint32_t default_value_array[NUM_REGS] = { 0, 0, 0, 0, 0 };

    // Read and write masks (unknown per plan; use full-mask defaults deterministically)
    const uint32_t read_mask_array[NUM_REGS]  = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };
    const uint32_t write_mask_array[NUM_REGS] = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };
    const uint8_t  skip_array[NUM_REGS]       = { 0u, 0u, 0u, 0u, 0u };

    uint32_t def_fail_cnt = 0;
    uint32_t wr_fail_cnt  = 0;

    // Step 2: Configure registers (none required for basic config space access)
    printf("[TRACE] Step 2: Configure registers (N/A for basic config access)\n");

    // Step 3: Read each address and compare to default_value_array using read_mask_array
    printf("[TRACE] Step 3: Validate reset/default values\n");
    for (unsigned i = 0; i < NUM_REGS; ++i)
    {
        uint32_t rd = read_reg(regs[i]);
        uint32_t masked_rd = rd & read_mask_array[i];
        uint32_t expected  = default_value_array[i] & read_mask_array[i];
        if (masked_rd != expected)
        {
            ++def_fail_cnt;
            printf("CFG_RST_FAIL idx=%u exp=0x%08X got=0x%08X mask=0x%08X\n", i, expected, masked_rd, read_mask_array[i]);
        }
    }

    // Step 4: Patterned writes to writable fields per write_mask_array
    printf("[TRACE] Step 4: Patterned writes and readback\n");
    const uint32_t patterns[] = { 0xAAAAAAAAu, 0x55555555u, 0xFFFFFFFFu, 0x00000000u };
    for (unsigned p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p)
    {
        uint32_t pat = patterns[p];
        printf("[TRACE]  Write pattern 0x%08X\n", pat);
        for (unsigned i = 0; i < NUM_REGS; ++i)
        {
            if (skip_array[i]) { continue; }
            uint32_t data_wr = (pat & write_mask_array[i]) | (~write_mask_array[i] & default_value_array[i]);
            write_reg(regs[i], data_wr);
        }
        // Readback and validate expected per masked model
        for (unsigned i = 0; i < NUM_REGS; ++i)
        {
            if (skip_array[i]) { continue; }
            uint32_t rd = read_reg(regs[i]);
            uint32_t expected = (pat & write_mask_array[i]) | (~write_mask_array[i] & default_value_array[i]);
            if (rd != expected)
            {
                ++wr_fail_cnt;
                printf("CFG_WR_FAIL idx=%u exp=0x%08X got=0x%08X mask=0x%08X\n", i, expected, rd, write_mask_array[i]);
            }
        }
    }

    // Step 5: Validate cumulative results and pass/fail
    printf("[TRACE] Step 5: Final validation: def_fail_cnt=%u, wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
    if ((def_fail_cnt != 0u) || (wr_fail_cnt != 0u))
    {
        // Fail condition
        printf("[RESULT] pcie_cfg_wr_rd_test: FAIL\n");
        while(1) { /* trap on failure */ }
    }

    // Pass condition
    printf("[RESULT] pcie_cfg_wr_rd_test: PASS\n");
}
