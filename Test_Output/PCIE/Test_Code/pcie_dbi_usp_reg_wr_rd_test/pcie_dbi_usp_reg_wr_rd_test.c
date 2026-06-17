#include "pcie_dbi_usp_reg_wr_rd_test.h"
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

/* Register Definitions (Partial RAG; offsets unknown -> placeholders) */
#define PCIE_APP_BASE     0x00000000  // TODO: Replace with PCIe Application Register base

// RAG data not available, using placeholder
#define GPR0_OFFSET       0x00000000  // TODO: Replace with actual offset
#define GPR1_OFFSET       (GPR0_OFFSET + 0x4)  // TODO: Confirm actual offset
#define GPR2_OFFSET       (GPR0_OFFSET + 0x8)  // TODO: Confirm actual offset

#define DEFAULT_GPR0      0x00000000
#define DEFAULT_GPR1      0x00000000
#define DEFAULT_GPR2      0x00000000

#define GPR_RW_MASK       0xFFFFFFFFu

static inline uint32_t app_addr(uint32_t offset)
{
    return (PCIE_APP_BASE + offset);
}

void pcie_dbi_usp_reg_wr_rd_test(void)
{
    printf("Running %s\n", "pcie_dbi_usp_reg_wr_rd_test");

    const char *names[] = { "GPR0", "GPR1", "GPR2" };
    const uint32_t offsets[] = { GPR0_OFFSET, GPR1_OFFSET, GPR2_OFFSET };
    const uint32_t def_vals[] = { DEFAULT_GPR0, DEFAULT_GPR1, DEFAULT_GPR2 };

    uint32_t def_fail_cnt = 0;
    uint32_t wr_fail_cnt  = 0;

    /* Step 1/2: Default read and compare to expected (all zeros by spec) */
    for (unsigned i = 0; i < 3; ++i)
    {
        uint32_t val = read_reg(app_addr(offsets[i]));
        uint32_t exp = def_vals[i];
        if ((val & GPR_RW_MASK) != (exp & GPR_RW_MASK))
        {
            printf("USP_RST_FAIL: %s exp=0x%08X got=0x%08X\n", names[i], exp, val);
            def_fail_cnt++;
        }
    }

    /* Step 3/4/5: Write fixed pattern 0xA5A5A5A5, read back and compare */
    const uint32_t pat = 0xA5A5A5A5u;

    for (unsigned i = 0; i < 3; ++i)
    {
        uint32_t addr = app_addr(offsets[i]);
        write_reg(addr, pat);
        uint32_t rv = read_reg(addr) & GPR_RW_MASK;
        if (rv != (pat & GPR_RW_MASK))
        {
            printf("USP_WR_FAIL: %s pat=0x%08X got=0x%08X\n", names[i], pat, rv);
            wr_fail_cnt++;
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
