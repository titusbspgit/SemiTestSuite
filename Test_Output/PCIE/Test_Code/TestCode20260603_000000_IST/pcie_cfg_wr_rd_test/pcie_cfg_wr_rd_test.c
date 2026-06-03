#include "pcie_cfg_wr_rd_test.h"
#include <stdint.h>

/* RAG-derived register bases, offsets, and fields */
/* Base: CONFIG_SPACE_BASE (from RAG) */
#define BASE_ADDR_VENDOR_DEVICE_ID   CONFIG_SPACE_BASE
#define OFFSET_VENDOR_DEVICE_ID      0x000u
#define VENDOR_DEVICE_ID_VENDOR_ID_SHIFT   0u
#define VENDOR_DEVICE_ID_VENDOR_ID_MASK    (0xFFFFu << VENDOR_DEVICE_ID_VENDOR_ID_SHIFT)
#define VENDOR_DEVICE_ID_DEVICE_ID_SHIFT   16u
#define VENDOR_DEVICE_ID_DEVICE_ID_MASK    (0xFFFFu << VENDOR_DEVICE_ID_DEVICE_ID_SHIFT)

#define BASE_ADDR_STATUS_COMMAND     CONFIG_SPACE_BASE
#define OFFSET_STATUS_COMMAND        0x004u
#define STATUS_COMMAND_COMMAND_SHIFT 0u
#define STATUS_COMMAND_COMMAND_MASK  (0xFFFFu << STATUS_COMMAND_COMMAND_SHIFT)
#define STATUS_COMMAND_STATUS_SHIFT  16u
#define STATUS_COMMAND_STATUS_MASK   (0xFFFFu << STATUS_COMMAND_STATUS_SHIFT)

#define BASE_ADDR_CLASSCODE          CONFIG_SPACE_BASE
#define OFFSET_CLASSCODE             0x008u
#define CLASSCODE_REVISION_ID_SHIFT  0u
#define CLASSCODE_REVISION_ID_MASK   (0xFFu << CLASSCODE_REVISION_ID_SHIFT)
#define CLASSCODE_PROG_IF_SHIFT      8u
#define CLASSCODE_PROG_IF_MASK       (0xFFu << CLASSCODE_PROG_IF_SHIFT)
#define CLASSCODE_SUBCLASS_SHIFT     16u
#define CLASSCODE_SUBCLASS_MASK      (0xFFu << CLASSCODE_SUBCLASS_SHIFT)
#define CLASSCODE_BASE_CLASS_SHIFT   24u
#define CLASSCODE_BASE_CLASS_MASK    (0xFFu << CLASSCODE_BASE_CLASS_SHIFT)

#define BASE_ADDR_BAR0               CONFIG_SPACE_BASE
#define OFFSET_BAR0                  0x010u
#define BAR0_IO_SPACE_SHIFT          0u
#define BAR0_IO_SPACE_MASK           (0x1u << BAR0_IO_SPACE_SHIFT)
#define BAR0_MEM_TYPE_SHIFT          1u
#define BAR0_MEM_TYPE_MASK           (0x3u << BAR0_MEM_TYPE_SHIFT)
#define BAR0_PREFETCHABLE_SHIFT      3u
#define BAR0_PREFETCHABLE_MASK       (0x1u << BAR0_PREFETCHABLE_SHIFT)
#define BAR0_MEM_BASE_ADDR_SHIFT     4u
#define BAR0_MEM_BASE_ADDR_MASK      (0x0FFFFFFFu << BAR0_MEM_BASE_ADDR_SHIFT)
#define BAR0_IO_BASE_ADDR_SHIFT      2u
#define BAR0_IO_BASE_ADDR_MASK       (0x3FFFFFFFu << BAR0_IO_BASE_ADDR_SHIFT)

#define BASE_ADDR_INT_PIN            CONFIG_SPACE_BASE
#define OFFSET_INT_PIN               0x03Du /* 8-bit register per RAG */
#define INT_PIN_INT_PIN_SHIFT        0u
#define INT_PIN_INT_PIN_MASK         (0xFFu << INT_PIN_INT_PIN_SHIFT)

static inline void write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

/* Simple stub for finish() referenced in the plan */
static inline void finish(int status) { (void)status; }

void pcie_cfg_wr_rd_test(void)
{
    uint32_t val;
    uint32_t def_fail_cnt = 0u;
    uint32_t wr_fail_cnt  = 0u;

    /* Step 1: Enable clocks */
    /* Info not provided in plan/RAG; assumed done by platform init. */

    /* Step 2: Configure registers (from RAG) */
    /* No explicit config required by plan for these config-space registers. */

    /* Step 3/4/5: Read defaults, attempt writes with patterns, validate */
    const uint32_t patterns[] = { 0xAAAAAAAAu, 0x55555555u, 0xFFFFFFFFu, 0x00000000u };

    /* Helper macro to exercise a 32-bit register at (BASE + OFFSET) */
#define EXERCISE_REG(BASE, OFF, READ_MASK)                                \
    do {                                                                  \
        uint32_t addr__ = (uint32_t)((BASE) + (OFF));                     \
        uint32_t def__  = read_reg(addr__);                               \
        /* Debug: default read value */                                    \
        (void)def__;                                                       \
        /* Default validation against unknown golden is not possible       \
           without default_value_array; skipping def_fail_cnt updates. */  \
        for (unsigned p__ = 0; p__ < (sizeof(patterns)/sizeof(patterns[0])); ++p__) { \
            uint32_t wr__   = patterns[p__];                               \
            write_reg(addr__, wr__);                                       \
            uint32_t rd__   = read_reg(addr__);                            \
            uint32_t expect = wr__ & (READ_MASK);                          \
            uint32_t actual = rd__ & (READ_MASK);                          \
            if (actual != expect) {                                        \
                ++wr_fail_cnt;                                             \
                /* Debug: CFG_WR_FAIL at addr__ */                         \
            }                                                              \
        }                                                                  \
    } while (0)

    /* Use full-width masks per RAG bit widths. */
    EXERCISE_REG(BASE_ADDR_VENDOR_DEVICE_ID, OFFSET_VENDOR_DEVICE_ID, 0xFFFFFFFFu);
    EXERCISE_REG(BASE_ADDR_STATUS_COMMAND,   OFFSET_STATUS_COMMAND,   0xFFFFFFFFu);
    EXERCISE_REG(BASE_ADDR_CLASSCODE,        OFFSET_CLASSCODE,        0xFFFFFFFFu);
    EXERCISE_REG(BASE_ADDR_BAR0,             OFFSET_BAR0,             0xFFFFFFFFu);
    EXERCISE_REG(BASE_ADDR_INT_PIN,          OFFSET_INT_PIN,          0x000000FFu);

#undef EXERCISE_REG

    /* Finalize per plan */
    if ((def_fail_cnt == 0u) && (wr_fail_cnt == 0u)) {
        finish(0);
    } else {
        finish(1);
        while(1);
    }
}
