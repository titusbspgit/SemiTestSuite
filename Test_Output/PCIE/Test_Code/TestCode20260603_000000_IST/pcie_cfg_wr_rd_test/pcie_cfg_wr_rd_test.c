#include "pcie_cfg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */
// RAG used: standard PCIe configuration space register offsets are known.
// Base address is device-specific and not available in RAG; using placeholder.
// TODO: Replace BASE_ADDR_PCIE_CFG with actual PCIe Local Config Space base address
#define BASE_ADDR_PCIE_CFG              0x0  // TODO: Update

// Offsets within configuration space (from RAG/spec)
#define VENDOR_DEVICE_ID_OFFSET         0x00  // [31:16]=DEVICE_ID, [15:0]=VENDOR_ID (RO)
#define STATUS_COMMAND_OFFSET           0x04  // [31:16]=STATUS (RW1C/RO), [15:0]=COMMAND (RW)
#define REV_ID_CLASS_CODE_OFFSET        0x08  // [31:8]=CLASS CODE fields (RO), [7:0]=REV ID (RO)
#define BAR0_OFFSET                     0x10  // Base Address Register 0 (RW)
#define INTLINE_INTPIN_OFFSET           0x3C  // [7:0]=INT LINE (RW), [15:8]=INT PIN (RO)

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
void pcie_cfg_wr_rd_test(void)
{
    printf("Running %s\n", "pcie_cfg_wr_rd_test");

    // Step 1: Based on Excel steps - arrays of addresses and masks
    const uint32_t cfg_offsets[] = {
        VENDOR_DEVICE_ID_OFFSET,
        STATUS_COMMAND_OFFSET,
        REV_ID_CLASS_CODE_OFFSET,
        BAR0_OFFSET,
        INTLINE_INTPIN_OFFSET
    };

    // Default values hardcoded to 0 per Excel note
    const uint32_t default_value_array[] = {
        0x00000000, // VENDOR_DEVICE_ID (RO)
        0x00000000, // STATUS_COMMAND
        0x00000000, // REV_ID_CLASS_CODE (RO)
        0x00000000, // BAR0
        0x00000000  // INT LINE/PIN
    };

    // Read mask controls which bits are checked for reset value
    const uint32_t read_mask_array[] = {
        0xFFFFFFFF, // VENDOR_DEVICE_ID (compare full 32 bits)
        0x0000FFFF, // Only COMMAND default; STATUS bits may be RW1C/implementation-defined
        0xFFFFFFFF, // REV_ID_CLASS_CODE (typically RO)
        0xFFFFFFFF, // BAR0
        0x000000FF  // Only INT LINE is typically RW; INT PIN is RO and device-specific
    };

    // Write mask controls writable fields per Excel logic
    const uint32_t write_mask_array[] = {
        0x00000000, // VENDOR/DEVICE ID are RO
        0x0000FFFF, // COMMAND is writable; avoid STATUS (upper 16) as it is largely RW1C/RO
        0x00000000, // CLASS CODE/REV ID are RO
        0xFFFFFFFF, // BAR0 writable (may have side effects in HW)
        0x000000FF  // Interrupt Line field only
    };

    // skip_array present in Excel; default to no-skip
    const uint8_t skip_array[] = {0,0,0,0,0};

    const uint32_t patterns[] = {0xAAAAAAAAu, 0x55555555u, 0xFFFFFFFFu, 0x00000000u};

    unsigned def_fail_cnt = 0;
    unsigned wr_fail_cnt = 0;

    // Step 1: Read each address and compare masked default
    for (unsigned i = 0; i < sizeof(cfg_offsets)/sizeof(cfg_offsets[0]); ++i)
    {
        if (skip_array[i]) continue;
        uint32_t addr = BASE_ADDR_PCIE_CFG + cfg_offsets[i];
        uint32_t val = read_reg(addr);
        uint32_t masked = val & read_mask_array[i];
        uint32_t expected = default_value_array[i] & read_mask_array[i];
        if (masked != expected)
        {
            ++def_fail_cnt;
            printf("CFG_RST_FAIL: index=%u addr=0x%08X read=0x%08X exp=0x%08X mask=0x%08X\n",
                   i, addr, masked, expected, read_mask_array[i]);
        }
    }

    // Step 2/3: For each test pattern, write to writable addresses and verify
    for (unsigned p = 0; p < sizeof(patterns)/sizeof(patterns[0]); ++p)
    {
        uint32_t pat = patterns[p];
        // Write phase
        for (unsigned i = 0; i < sizeof(cfg_offsets)/sizeof(cfg_offsets[0]); ++i)
        {
            if (skip_array[i]) continue;
            if (write_mask_array[i] == 0) continue; // read-only
            uint32_t addr = BASE_ADDR_PCIE_CFG + cfg_offsets[i];
            uint32_t cur = read_reg(addr);
            uint32_t to_write = (cur & ~write_mask_array[i]) | (pat & write_mask_array[i]);
            write_reg(addr, to_write);
        }
        // Readback/validate
        for (unsigned i = 0; i < sizeof(cfg_offsets)/sizeof(cfg_offsets[0]); ++i)
        {
            if (skip_array[i]) continue;
            uint32_t addr = BASE_ADDR_PCIE_CFG + cfg_offsets[i];
            uint32_t val = read_reg(addr);
            uint32_t expected = ( (val & ~write_mask_array[i]) | (pat & write_mask_array[i]) );
            // Compare only the writable portion for validation
            uint32_t val_w = val & write_mask_array[i];
            uint32_t exp_w = pat & write_mask_array[i];
            if (val_w != exp_w)
            {
                ++wr_fail_cnt;
                printf("CFG_WR_FAIL: index=%u addr=0x%08X read=0x%08X exp=0x%08X wmask=0x%08X pattern=0x%08X\n",
                       i, addr, val_w, exp_w, write_mask_array[i], pat);
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
