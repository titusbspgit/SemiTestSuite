#include "pcie_dbi_dsp_reg_wr_rd_test.h"
#include <stdint.h>
#include "vendor/unity/unity.h"
#include "mock/mock_mmio.h"
#include "include/pcie_addr_map.h"

void pcie_dbi_dsp_reg_wr_rd_test(void)
{
    uint32_t addrs[] = {
        PCIE_DBI_DSP_CMD_STATUS_ADDR,
        PCIE_DBI_DSP_CFG_SETUP_ADDR,
        PCIE_DBI_DSP_OB_SIZE_ADDR,
        PCIE_DBI_DSP_IRQ_EOI_ADDR
    };
    const uint32_t patterns[] = {0xAAAAAAAAu, 0x55555555u, 0x00000000u};

    mmio_init();

    unsigned def_fail_cnt = 0, wr_fail_cnt = 0;

    /* Step 1: Defaults assumed 0 in mock */
    for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
        uint32_t v = mmio_read32(addrs[i]);
        if (v != 0u) { ++def_fail_cnt; }
    }

    /* Step 2/3: Write patterns to all and verify last written */
    for (unsigned p = 0; p < sizeof(patterns)/sizeof(patterns[0]); ++p) {
        for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
            mmio_write32(addrs[i], patterns[p]);
        }
    }
    for (unsigned i = 0; i < sizeof(addrs)/sizeof(addrs[0]); ++i) {
        uint32_t v = mmio_read32(addrs[i]);
        if (v != patterns[sizeof(patterns)/sizeof(patterns[0]) - 1]) { ++wr_fail_cnt; }
    }

    if (def_fail_cnt != 0 || wr_fail_cnt != 0) {
        TEST_FAIL_MESSAGE("DSP_RST_FAIL or DSP_WR_FAIL detected");
    }

    TEST_PASS();
}
