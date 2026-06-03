#include "pcie_cfg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */
/* RAG data used: Standard PCI/PCIe Configuration Space offsets.
 * Base address is device-specific. Placeholder provided below.
 */
#define PCIE_CFG_BASE                 0x0  /* TODO: Update with device-specific local config base or ECAM base */

#define VENDOR_DEVICE_ID_OFFSET       0x000
#define STATUS_COMMAND_OFFSET         0x004
#define CLASSCODE_OFFSET              0x008
#define BAR0_OFFSET                   0x010
#define INT_PIN_OFFSET                0x03C  /* Access 32-bit at 0x03C; INT_PIN is bits [15:8] */

#define BASE_ADDR_VENDOR_DEVICE_ID    (PCIE_CFG_BASE + VENDOR_DEVICE_ID_OFFSET)
#define BASE_ADDR_STATUS_COMMAND      (PCIE_CFG_BASE + STATUS_COMMAND_OFFSET)
#define BASE_ADDR_CLASSCODE           (PCIE_CFG_BASE + CLASSCODE_OFFSET)
#define BASE_ADDR_BAR0                (PCIE_CFG_BASE + BAR0_OFFSET)
#define BASE_ADDR_INT_PIN             (PCIE_CFG_BASE + INT_PIN_OFFSET)

/* Bitfield helpers from RAG (subset) */
#define STATUS_COMMAND_COMMAND_MEM_EN    (1u << 1)
#define STATUS_COMMAND_COMMAND_BUS_MASTER_EN (1u << 2)
#define INT_PIN_FIELD_MASK               (0xFFu << 8)

/* Low-level access */
static inline void write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

/* Minimal finish() shim to preserve test intent */
static inline void finish(int code)
{
    printf("finish(%d)\n", code);
}

/* Test Function */
void pcie_cfg_wr_rd_test(void)
{
    uint32_t val;
    unsigned i, p;
    unsigned def_fail_cnt = 0, wr_fail_cnt = 0;

    printf("Running %s\n", "pcie_cfg_wr_rd_test");

    /* Address list derived from Excel Impacted Registers */
    static const uint32_t addr_array[] = {
        BASE_ADDR_VENDOR_DEVICE_ID,
        BASE_ADDR_STATUS_COMMAND,
        BASE_ADDR_CLASSCODE,
        BASE_ADDR_BAR0,
        BASE_ADDR_INT_PIN
    };

    /* Default reset values (Excel note: hardcoded to 0).
     * TODO: Replace with true reset defaults from device manual if available.
     */
    static const uint32_t default_value_array[] = {
        0x00000000, /* VENDOR_DEVICE_ID (device-specific; RAG indicates not 0) */
        0x00000000, /* STATUS_COMMAND: commonly COMMAND=0x0000; STATUS device-specific */
        0x00000000, /* CLASSCODE (device-specific) */
        0x00000000, /* BAR0 */
        0x00000000  /* INT/Latency dword */
    };

    /* Read and write masks (Excel: uses read_mask_array/write_mask_array/skip_array).
     * Values below are safe defaults; update as per HW RO/RW behavior.
     * - VENDOR_DEVICE_ID: RO -> write mask 0
     * - STATUS_COMMAND: write COMMAND[15:0] only
     * - CLASSCODE: RO -> write mask 0
     * - BAR0: RW by RC during enumeration -> full mask
     * - INT_PIN: target INT_PIN in [15:8]
     */
    static const uint32_t read_mask_array[] = {
        0xFFFFFFFFu, /* VENDOR_DEVICE_ID */
        0xFFFFFFFFu, /* STATUS_COMMAND */
        0xFFFFFFFFu, /* CLASSCODE */
        0xFFFFFFFFu, /* BAR0 */
        0xFFFFFFFFu  /* INT/Latency dword */
    };

    static const uint32_t write_mask_array[] = {
        0x00000000u, /* VENDOR_DEVICE_ID */
        0x0000FFFFu, /* STATUS_COMMAND: COMMAND field */
        0x00000000u, /* CLASSCODE */
        0xFFFFFFFFu, /* BAR0 */
        0x0000FF00u  /* INT_PIN field within 0x03C */
    };

    /* Patterns per Excel */
    static const uint32_t patterns[] = {
        0xAAAAAAAAu, 0x55555555u, 0xFFFFFFFFu, 0x00000000u
    };

    /* Step 1: Read each address and compare to default_value_array using read_mask_array */
    for (i = 0; i < (sizeof(addr_array)/sizeof(addr_array[0])); ++i)
    {
        uint32_t exp = default_value_array[i] & read_mask_array[i];
        val = read_reg(addr_array[i]) & read_mask_array[i];
        if (val != exp)
        {
            ++def_fail_cnt;
            printf("CFG_RST_FAIL: idx=%u addr=0x%08X exp=0x%08X act=0x%08X\n", i, addr_array[i], exp, val);
        }
    }

    /* Step 2-3: Write patterns to writable addresses per write_mask_array and verify */
    for (p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p)
    {
        uint32_t data_wr = patterns[p];
        for (i = 0; i < (sizeof(addr_array)/sizeof(addr_array[0])); ++i)
        {
            uint32_t wmask = write_mask_array[i];
            if (wmask == 0u)
            {
                continue; /* skip RO/registers */
            }
            uint32_t computed = (data_wr & wmask) | ((~wmask) & default_value_array[i]);
            write_reg(addr_array[i], computed);
            val = read_reg(addr_array[i]) & read_mask_array[i];
            /* Expected follows Excel formula */
            uint32_t expected = (data_wr & wmask) | ((~wmask) & default_value_array[i]);
            expected &= read_mask_array[i];
            if (val != expected)
            {
                ++wr_fail_cnt;
                printf("CFG_WR_FAIL: idx=%u pat=0x%08X addr=0x%08X exp=0x%08X act=0x%08X\n", i, data_wr, addr_array[i], expected, val);
            }
        }
    }

    /* Step 4-5: Validation and finish */
    if (def_fail_cnt == 0 && wr_fail_cnt == 0)
    {
        printf("Test Passed\n");
        finish(0);
    }
    else
    {
        printf("Test Failed\n");
        finish(1);
    }
}
