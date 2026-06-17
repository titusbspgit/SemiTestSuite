#include "pcie_cfg_wr_rd_test.h"
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
// CFG_BASE is the base of PCIe Type-0 Configuration Space mapping for the target function
#define CFG_BASE                 0x00000000  // TODO: Replace with actual config space base

#define VENDOR_DEVICE_ID_OFFSET  0x00000000  // From PCIe Type-0 header
#define STATUS_COMMAND_OFFSET    0x00000004  // STATUS[31:16] | COMMAND[15:0]
#define CLASSCODE_OFFSET         0x00000008  // REV_ID[7:0] | PIF[15:8] | SUB_CLASS[23:16] | BASE_CLASS[31:24]
#define BAR0_OFFSET              0x00000010  // BAR0 (32b or 64b lower)
#define INT_LINE_PIN_OFFSET      0x0000003C  // INT_LINE @ [7:0], INT_PIN @ [15:8] (PCI), or [31:24] per RAG note

/* Reset defaults (unknown -> placeholders) */
#define DEFAULT_VENDOR_DEVICE_ID 0x00000000  // TODO: Replace with silicon-specific default
#define DEFAULT_STATUS_COMMAND   0x00000000
#define DEFAULT_CLASSCODE        0x00000000  // TODO: Replace with device class/rev
#define DEFAULT_BAR0             0x00000000
#define DEFAULT_INT_PIN_WORD     0x00000000  // TODO: INT_PIN byte encoded within this word

/* Masks from RAG */
#define VENDOR_DEVICE_ID_RD_MASK 0xFFFFFFFFu
#define VENDOR_DEVICE_ID_WR_MASK 0x00000000u

#define STATUS_COMMAND_RD_MASK   0xFFFFFFFFu
#define STATUS_COMMAND_WR_MASK   0x00000406u   /* COMMAND bits RW via cfg: BME,MSE,INTxDIS */
#define STATUS_W1C_MASK          0xBE000000u   /* Device-specific; treat as W1C per RAG note */

#define CLASSCODE_RD_MASK        0xFFFFFFFFu
#define CLASSCODE_WR_MASK        0x00000000u

#define BAR0_RD_MASK             0xFFFFFFFFu   /* Readback may reflect size probe when writing 0xFFFF_FFFF */
#define BAR0_WR_MASK             0xFFFFFFF0u   /* Attribute bits fixed; BASE[31:4] RW */

#define INT_PIN_RD_MASK          0x0000FF00u   /* INT_PIN typically at [15:8] for legacy PCI; device-specific */
#define INT_PIN_WR_MASK          0x00000000u

static inline uint32_t cfg_addr(uint32_t offset)
{
    return (CFG_BASE + offset);
}

void pcie_cfg_wr_rd_test(void)
{
    printf("Running %s\n", "pcie_cfg_wr_rd_test");

    /* Step 1: Initialize arrays based on Excel steps */
    const char *names[] = {
        "VENDOR_DEVICE_ID",
        "STATUS_COMMAND",
        "CLASSCODE",
        "BAR0",
        "INT_PIN"
    };
    const uint32_t offsets[] = {
        VENDOR_DEVICE_ID_OFFSET,
        STATUS_COMMAND_OFFSET,
        CLASSCODE_OFFSET,
        BAR0_OFFSET,
        INT_LINE_PIN_OFFSET
    };
    const uint32_t def_vals[] = {
        DEFAULT_VENDOR_DEVICE_ID,
        DEFAULT_STATUS_COMMAND,
        DEFAULT_CLASSCODE,
        DEFAULT_BAR0,
        DEFAULT_INT_PIN_WORD
    };
    const uint32_t rd_masks[] = {
      VENDOR_DEVICE_ID_RD_MASK,
      STATUS_COMMAND_RD_MASK,
      CLASSCODE_RD_MASK,
      BAR0_RD_MASK,
      INT_PIN_RD_MASK
    };
    const uint32_t wr_masks[] = {
      VENDOR_DEVICE_ID_WR_MASK,
      STATUS_COMMAND_WR_MASK,
      CLASSCODE_WR_MASK,
      BAR0_WR_MASK,
      INT_PIN_WR_MASK
    };

    uint32_t def_fail_cnt = 0;
    uint32_t wr_fail_cnt  = 0;

    /* Step 2: Default read and compare against defaults using read masks */
    for (unsigned i = 0; i < (sizeof(offsets)/sizeof(offsets[0])); ++i)
    {
        if (rd_masks[i] == 0)
            continue; /* Skip if no readable bits */

        uint32_t val = read_reg(cfg_addr(offsets[i]));
        uint32_t exp = def_vals[i];
        if ((val & rd_masks[i]) != (exp & rd_masks[i]))
        {
            printf("CFG_RST_FAIL: %s exp=0x%08X got=0x%08X mask=0x%08X\n",
                   names[i], (exp & rd_masks[i]), (val & rd_masks[i]), rd_masks[i]);
            def_fail_cnt++;
        }
    }

    /* Step 3/4: Write patterns using write masks and compute expected */
    const uint32_t patterns[] = { 0xAAAAAAAAu, 0x55555555u, 0xFFFFFFFFu, 0x00000000u };

    for (unsigned p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p)
    {
        for (unsigned i = 0; i < (sizeof(offsets)/sizeof(offsets[0])); ++i)
        {
            uint32_t wmask = wr_masks[i];
            if (wmask == 0)
                continue; /* RO register, skip write/readback */

            uint32_t addr = cfg_addr(offsets[i]);
            uint32_t defv = def_vals[i];
            uint32_t pat  = patterns[p];

            /* Program value with mask applied */
            uint32_t writev = (defv & ~wmask) | (pat & wmask);
            write_reg(addr, writev);

            /* Read back and validate only the writable bits */
            uint32_t rval = read_reg(addr);
            uint32_t exp  = writev;

            /* Special handling: STATUS_COMMAND has W1C status bits in upper half; ensure they read 0 if written 1 */
            if (i == 1) /* STATUS_COMMAND index */
            {
                /* Any 1 written to W1C bits clears them -> expected 0 for those bits */
                uint32_t w1c_ones = (pat & STATUS_W1C_MASK);
                (void)w1c_ones; /* informational; exp already excludes STATUS W1C bits due to WR mask only covering COMMAND */
                /* We restrict comparison to COMMAND write mask only */
                rval &= STATUS_COMMAND_WR_MASK;
                exp  &= STATUS_COMMAND_WR_MASK;
            }
            else if (i == 3) /* BAR0 size-probe caveat when pat == 0xFFFFFFFF */
            {
                if (pat == 0xFFFFFFFFu)
                {
                    /* Readback during size probe is implementation-defined; compare only attribute bits */
                    rval &= ~BAR0_WR_MASK; /* Attributes are RO/fixed */
                    exp  &= ~BAR0_WR_MASK;
                }
                else
                {
                    rval &= BAR0_WR_MASK;
                    exp  &= BAR0_WR_MASK;
                }
            }
            else
            {
                rval &= wmask;
                exp  &= wmask;
            }

            if (rval != exp)
            {
                printf("CFG_WR_FAIL: %s pat=0x%08X exp=0x%08X got=0x%08X mask=0x%08X\n",
                       names[i], pat, exp, rval, wmask);
                wr_fail_cnt++;
            }
        }
    }

    /* Step 5/6: Final decision */
    if (def_fail_cnt > 0 || wr_fail_cnt > 0)
    {
        printf("Failures: def=%u, wr=%u\n", def_fail_cnt, wr_fail_cnt);
        printf("Test Failed\n");
        while(1); /* exact sequence required */
        finish(1);
        return;
    }

    printf("Test Passed\n");
    finish(0);
}
