#include "pcie_dbi_dsp_reg_wr_rd_test.h"
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

/* Register Definitions (RAG-enriched; base is device-specific) */
#define PCIE_APP_BASE        0x00000000  // TODO: Replace with PCIe Application Register base

#define CMD_STATUS_OFFSET    0x00000004  // R/W1C status bits at [31:16]
#define CFG_SETUP_OFFSET     0x00000110  // RW header/programming fields
#define OB_SIZE_OFFSET       0x00000030  // RW, size encoding in [5:0]
#define IRQ_EOI_OFFSET       0x00000050  // WO [7:0]

/* Reset defaults (placeholders where unknown) */
#define DEFAULT_CMD_STATUS   0x00000000
#define DEFAULT_CFG_SETUP    0x00000000
#define DEFAULT_OB_SIZE      0x00000000
#define DEFAULT_IRQ_EOI      0x00000000  /* Readback not meaningful (WO) */

/* Masks from RAG */
#define CMD_STATUS_RD_MASK   0xFEFFFFFFu  /* reserved read 0; practical check uses W1C and implemented bits */
#define CMD_STATUS_W1C_MASK  0xBE000000u  /* bits that clear on write-1 */
#define CMD_STATUS_RW_MASK   0xBE000000u  /* writable via W1C semantics */

#define CFG_SETUP_RD_MASK    0x80FFFFFCu
#define CFG_SETUP_RW_MASK    0x80FFFFFCu

#define OB_SIZE_RD_MASK      0x0000003Fu
#define OB_SIZE_RW_MASK      0x0000003Fu

#define IRQ_EOI_RD_MASK      0x00000000u  /* WO field */
#define IRQ_EOI_RW_MASK      0x000000FFu

static inline uint32_t app_addr(uint32_t offset)
{
    return (PCIE_APP_BASE + offset);
}

void pcie_dbi_dsp_reg_wr_rd_test(void)
{
    printf("Running %s\n", "pcie_dbi_dsp_reg_wr_rd_test");

    const char *names[] = {
        "CMD_STATUS",
        "CFG_SETUP",
        "OB_SIZE",
        "IRQ_EOI"
    };
    const uint32_t offsets[] = {
        CMD_STATUS_OFFSET,
        CFG_SETUP_OFFSET,
        OB_SIZE_OFFSET,
        IRQ_EOI_OFFSET
    };
    const uint32_t def_vals[] = {
        DEFAULT_CMD_STATUS,
        DEFAULT_CFG_SETUP,
        DEFAULT_OB_SIZE,
        DEFAULT_IRQ_EOI
    };
    const uint32_t rd_masks[] = {
        CMD_STATUS_RD_MASK,
        CFG_SETUP_RD_MASK,
        OB_SIZE_RD_MASK,
        IRQ_EOI_RD_MASK
    };
    const uint32_t rw_masks[] = {
        CMD_STATUS_RW_MASK,
        CFG_SETUP_RW_MASK,
        OB_SIZE_RW_MASK,
        IRQ_EOI_RW_MASK
    };

    uint32_t def_fail_cnt = 0;
    uint32_t wr_fail_cnt  = 0;

    /* Step 1/2: Default read/compare */
    for (unsigned i = 0; i < (sizeof(offsets)/sizeof(offsets[0])); ++i)
    {
        if (rd_masks[i] == 0)
            continue; /* skip WO registers like IRQ_EOI */
        uint32_t val = read_reg(app_addr(offsets[i]));
        uint32_t exp = def_vals[i] & rd_masks[i];
        if ((val & rd_masks[i]) != exp)
        {
            printf("DSP_RST_FAIL: %s exp=0x%08X got=0x%08X mask=0x%08X\n",
                   names[i], exp, (val & rd_masks[i]), rd_masks[i]);
            def_fail_cnt++;
        }
    }

    /* Step 3/4/5: Write patterns, read back, compare */
    const uint32_t patterns[] = { 0xAAAAAAAAu, 0x55555555u, 0x00000000u };

    for (unsigned p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p)
    {
        for (unsigned i = 0; i < (sizeof(offsets)/sizeof(offsets[0])); ++i)
        {
            uint32_t mask = rw_masks[i];
            uint32_t addr = app_addr(offsets[i]);
            uint32_t pat  = patterns[p];
            if (mask == 0)
            {
                /* Write-only or non-readable; still perform write but skip compare */
                uint32_t wv = (def_vals[i] & ~mask) | (pat & mask);
                write_reg(addr, wv);
                continue;
            }

            /* Handle W1C semantics for CMD_STATUS */
            if (i == 0) /* CMD_STATUS index */
            {
                /* Write the pattern; any 1's in W1C bits will clear to 0 on readback */
                uint32_t w1c = CMD_STATUS_W1C_MASK;
                uint32_t wv  = (def_vals[i] & ~w1c) | (pat & w1c); /* writing 1 to W1C attempts to clear */
                write_reg(addr, wv);

                uint32_t rv = read_reg(addr) & (CMD_STATUS_RD_MASK);
                /* Expected: bits written with 1 in W1C clear to 0; others remain 0 from reset for this test */
                uint32_t exp = (def_vals[i] & ~ (pat & w1c)) & CMD_STATUS_RD_MASK;
                if (rv != exp)
                {
                    printf("DSP_WR_FAIL: %s pat=0x%08X exp=0x%08X got=0x%08X mask=0x%08X\n",
                           names[i], pat, exp, rv, CMD_STATUS_RD_MASK);
                    wr_fail_cnt++;
                }
            }
            else
            {
                uint32_t wv = (def_vals[i] & ~mask) | (pat & mask);
                write_reg(addr, wv);
                uint32_t rv = read_reg(addr) & mask;
                uint32_t exp = wv & mask;
                if (rv != exp)
                {
                    printf("DSP_WR_FAIL: %s pat=0x%08X exp=0x%08X got=0x%08X mask=0x%08X\n",
                           names[i], pat, exp, rv, mask);
                    wr_fail_cnt++;
                }
            }
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
