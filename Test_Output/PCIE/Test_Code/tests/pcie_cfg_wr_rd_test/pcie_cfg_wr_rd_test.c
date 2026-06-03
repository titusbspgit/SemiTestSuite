#include "pcie_cfg_wr_rd_test.h"
#include <stdint.h>
#include "vendor/unity/unity.h"
#include "mock/mock_mmio.h"
#include "include/pcie_addr_map.h"

void pcie_cfg_wr_rd_test(void)
{
#ifndef PCIE_ADDR_MAP_PROVIDED
    TEST_IGNORE_MESSAGE("Requires device-specific address map and RO/RW masks; provide pcie_addr_map.h overrides.");
#else
    uint32_t addrs[] = {
        PCIE_CFG_VENDOR_DEVICE_ID_ADDR,
        PCIE_CFG_STATUS_COMMAND_ADDR,
        PCIE_CFG_CLASSCODE_ADDR,
        PCIE_CFG_BAR0_ADDR,
        PCIE_CFG_INT_PIN_ADDR
    };
    uint32_t rd_masks[] = {
        VENDOR_DEVICE_ID_RD_MASK,
        STATUS_COMMAND_RD_MASK,
        CLASSCODE_RD_MASK,
        BAR0_RD_MASK,
        INT_PIN_RD_MASK
    };
    uint32_t wr_masks[] = {
        VENDOR_DEVICE_ID_WR_MASK,
        STATUS_COMMAND_WR_MASK,
        CLASSCODE_WR_MASK,
        BAR0_WR_MASK,
        INT_PIN_WR_MASK
    };

    const uint32_t patterns[] = {0xAAAAAAAAu, 0x55555555u, 0xFFFFFFFFu, 0x00000000u};

    mmio_init();

    unsigned def_fail_cnt = 0, wr_fail_cnt = 0;

    /* Step 1: Read defaults (assume 0 for mock unless overridden) */
    for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
        uint32_t v = mmio_read32(addrs[i]) & rd_masks[i];
        if (v != 0u) { ++def_fail_cnt; }
    }

    /* Step 2/3: Masked write and verify */
    for (unsigned p = 0; p < sizeof(patterns)/sizeof(patterns[0]); ++p) {
        for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
            mmio_write32_masked(addrs[i], patterns[p], wr_masks[i]);
        }
        for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
            uint32_t cur = mmio_read32(addrs[i]);
            uint32_t expected = (patterns[p] & wr_masks[i]);
            /* RO parts should retain previous (mock defaults to 0), emulate by AND with wr_mask and OR with ~wr_mask & 0 */
            if ((cur & (wr_masks[i] | rd_masks[i])) != (expected & (wr_masks[i] | rd_masks[i]))) {
                ++wr_fail_cnt;
            }
        }
    }

    if (def_fail_cnt != 0 || wr_fail_cnt != 0) {
        TEST_FAIL_MESSAGE("CFG_RST_FAIL or CFG_WR_FAIL detected");
    }

    TEST_PASS();
#endif
}
