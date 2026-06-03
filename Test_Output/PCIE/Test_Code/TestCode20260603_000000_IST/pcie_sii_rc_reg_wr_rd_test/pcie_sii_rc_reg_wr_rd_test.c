#include "pcie_sii_rc_reg_wr_rd_test.h"
#include <stdint.h>

/* RAG-derived SII RC application register mapping */
#define BASE_ADDR_IB_BAR0         RC_BAR0_BASE /* or RC_CFG_BASE + 0x800 via config window */
#define OFFSET_IB_BAR0            0x000u
#define IB_BAR0_EN_SHIFT          0u
#define IB_BAR0_EN_MASK           (0x1u << IB_BAR0_EN_SHIFT)

#define BASE_ADDR_IB_START_LO     RC_BAR0_BASE
#define OFFSET_IB_START_LO        0x008u
#define IB_START_LO_SHIFT         0u
#define IB_START_LO_MASK          (0xFFFFFFFFu << IB_START_LO_SHIFT)

#define BASE_ADDR_IB_OFFSET       RC_BAR0_BASE
#define OFFSET_IB_OFFSET          0x010u
#define IB_OFFSET_SHIFT           0u
#define IB_OFFSET_MASK            (0xFFFFFFFFu << IB_OFFSET_SHIFT)

static inline void write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

void pcie_sii_rc_reg_wr_rd_test(void)
{
    uint32_t val;

    /* Step 1: Enable clocks */
    /* Assumed done externally */

    /* Simple write/read patterns per plan */
    uint32_t addr_bar0   = (uint32_t)(BASE_ADDR_IB_BAR0   + OFFSET_IB_BAR0);
    uint32_t addr_start  = (uint32_t)(BASE_ADDR_IB_START_LO + OFFSET_IB_START_LO);
    uint32_t addr_offset = (uint32_t)(BASE_ADDR_IB_OFFSET + OFFSET_IB_OFFSET);

    /* Enable inbound window */
    write_reg(addr_bar0, IB_BAR0_EN_MASK);
    val = read_reg(addr_bar0);
    if ((val & IB_BAR0_EN_MASK) != IB_BAR0_EN_MASK) {
        while(1);
    }

    /* Pattern 0xFFFFFFFF then 0x00000000 */
    write_reg(addr_start, 0xFFFFFFFFu);
    val = read_reg(addr_start);
    if ((val & IB_START_LO_MASK) != (0xFFFFFFFFu & IB_START_LO_MASK)) {
        while(1);
    }

    write_reg(addr_start, 0x00000000u);
    val = read_reg(addr_start);
    if ((val & IB_START_LO_MASK) != 0x00000000u) {
        while(1);
    }

    write_reg(addr_offset, 0xFFFFFFFFu);
    val = read_reg(addr_offset);
    if ((val & IB_OFFSET_MASK) != (0xFFFFFFFFu & IB_OFFSET_MASK)) {
        while(1);
    }

    write_reg(addr_offset, 0x00000000u);
    val = read_reg(addr_offset);
    if ((val & IB_OFFSET_MASK) != 0x00000000u) {
        while(1);
    }

    /* Pass condition */
}
