#include "pcie_dbi_usp_reg_wr_rd_test.h"
#include <stdint.h>
#include "vendor/unity/unity.h"
#include "mock/mock_mmio.h"
#include "include/pcie_addr_map.h"

void pcie_dbi_usp_reg_wr_rd_test(void)
{
    uint32_t addrs[] = {
        PCIE_DBI_USP_GPR0_ADDR,
        PCIE_DBI_USP_GPR1_ADDR,
        PCIE_DBI_USP_GPR2_ADDR
    };
    const uint32_t pattern = 0xA5A5A5A5u;

    mmio_init();

    unsigned def_fail_cnt = 0, wr_fail_cnt = 0;

    /* Step 1: Defaults assumed 0 in mock */
    for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
        uint32_t v = mmio_read32(addrs[i]);
        if (v != 0u) { ++def_fail_cnt; }
    }

    /* Step 2: Write pattern */
    for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
        mmio_write32(addrs[i], pattern);
    }

    /* Step 3: Verify */
    for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
        uint32_t v = mmio_read32(addrs[i]);
        if (v != pattern) { ++wr_fail_cnt; }
    }

    if (def_fail_cnt != 0 || wr_fail_cnt != 0) {
        TEST_FAIL_MESSAGE("USP_RST_FAIL or USP_WR_FAIL detected");
    }

    TEST_PASS();
}
