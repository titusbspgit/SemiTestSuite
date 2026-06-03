#include "pcie_sii_rc_reg_wr_rd_test.h"
#include <stdint.h>
#include "vendor/unity/unity.h"
#include "mock/mock_mmio.h"
#include "include/pcie_addr_map.h"

void pcie_sii_rc_reg_wr_rd_test(void)
{
    uint32_t addrs[] = {
        PCIE_SII_RC_IB_BAR0_ADDR,
        PCIE_SII_RC_IB_START_LO_ADDR,
        PCIE_SII_RC_IB_OFFSET_ADDR
    };
    const uint32_t patterns[] = {0xFFFFFFFFu, 0x00000000u};

    mmio_init();

    unsigned def_fail_cnt = 0, wr_fail_cnt = 0;

    /* Step 1: Defaults assumed 0 */
    for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
        uint32_t v = mmio_read32(addrs[i]);
        if (v != 0u) { ++def_fail_cnt; }
    }

    /* Step 2: Write patterns */
    for (unsigned p = 0; p < sizeof(patterns)/sizeof(patterns[0]); ++p) {
        for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
            mmio_write32(addrs[i], patterns[p]);
        }
    }

    /* Step 3: Verify last pattern */
    for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
        uint32_t v = mmio_read32(addrs[i]);
        if (v != patterns[sizeof(patterns)/sizeof(patterns[0]) - 1]) { ++wr_fail_cnt; }
    }

    if (def_fail_cnt != 0 || wr_fail_cnt != 0) {
        TEST_FAIL_MESSAGE("SII_RC_RST_FAIL or SII_RC_WR_FAIL detected");
    }

    TEST_PASS();
}
