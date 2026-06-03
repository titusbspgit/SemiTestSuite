#include "pcie_cfg_wr_rd_test.h"
#include <stdint.h>

#ifndef PCIE_CFG_BASE
#error "PCIE_CFG_BASE is not defined. Provide PCIe configuration space base address."
#endif

// Offsets from RAG
#define VENDOR_DEVICE_ID_OFFSET   0x000u
#define STATUS_COMMAND_OFFSET     0x004u
#define CLASSCODE_OFFSET          0x008u
#define BAR0_OFFSET               0x010u
#define INT_PIN_OFFSET            0x03Du

// Absolute register addresses (Base + Offset)
#define BASE_ADDR_VENDOR_DEVICE_ID   (PCIE_CFG_BASE + VENDOR_DEVICE_ID_OFFSET)
#define BASE_ADDR_STATUS_COMMAND     (PCIE_CFG_BASE + STATUS_COMMAND_OFFSET)
#define BASE_ADDR_CLASSCODE          (PCIE_CFG_BASE + CLASSCODE_OFFSET)
#define BASE_ADDR_BAR0               (PCIE_CFG_BASE + BAR0_OFFSET)
#define BASE_ADDR_INT_PIN            (PCIE_CFG_BASE + INT_PIN_OFFSET)

// Field masks from RAG bit positions
// VENDOR_DEVICE_ID [31:16]=DEVICE_ID, [15:0]=VENDOR_ID
#define VENDOR_DEVICE_ID_DEVICE_ID_MASK   0xFFFF0000u
#define VENDOR_DEVICE_ID_VENDOR_ID_MASK   0x0000FFFFu
#define VENDOR_DEVICE_ID_ALL_MASK        (VENDOR_DEVICE_ID_DEVICE_ID_MASK | VENDOR_DEVICE_ID_VENDOR_ID_MASK)

// STATUS_COMMAND [31:16]=STATUS, [15:0]=COMMAND
#define STATUS_COMMAND_STATUS_MASK        0xFFFF0000u
#define STATUS_COMMAND_COMMAND_MASK       0x0000FFFFu
#define STATUS_COMMAND_ALL_MASK          (STATUS_COMMAND_STATUS_MASK | STATUS_COMMAND_COMMAND_MASK)

// CLASSCODE [31:24]=BASE_CLASS, [23:16]=SUB_CLASS, [15:8]=PROG_IF, [7:0]=REV_ID
#define CLASSCODE_BASE_CLASS_MASK         0xFF000000u
#define CLASSCODE_SUB_CLASS_MASK          0x00FF0000u
#define CLASSCODE_PROG_IF_MASK            0x0000FF00u
#define CLASSCODE_REV_ID_MASK             0x000000FFu
#define CLASSCODE_ALL_MASK               (CLASSCODE_BASE_CLASS_MASK | CLASSCODE_SUB_CLASS_MASK | CLASSCODE_PROG_IF_MASK | CLASSCODE_REV_ID_MASK)

// BAR0 fields
// BASE_ADDR_MEM [31:4], PREFETCHABLE [3], TYPE [2:1], MEM_IO [0], BASE_ADDR_IO [31:2]
#define BAR0_BASE_ADDR_MEM_MASK           0xFFFFFFF0u
#define BAR0_PREFETCHABLE_MASK            0x00000008u
#define BAR0_TYPE_MASK                    0x00000006u
#define BAR0_MEM_IO_MASK                  0x00000001u
#define BAR0_BASE_ADDR_IO_MASK            0xFFFFFFFCu
#define BAR0_ALL_MASK                    (BAR0_BASE_ADDR_MEM_MASK | BAR0_PREFETCHABLE_MASK | BAR0_TYPE_MASK | BAR0_MEM_IO_MASK | BAR0_BASE_ADDR_IO_MASK)

// INT_PIN [7:0]
#define INT_PIN_INT_PIN_MASK              0x000000FFu
#define INT_PIN_ALL_MASK                 (INT_PIN_INT_PIN_MASK)

static inline void write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

void pcie_cfg_wr_rd_test(void)
{
    // Step 1: Enable clocks (platform-specific; not provided by RAG/Excel)

    // Test patterns from Test Plan
    static const uint32_t patterns[] = { 0xAAAAAAAAu, 0x55555555u, 0xFFFFFFFFu, 0x00000000u };
    static const uint32_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);

    // Registers and masks strictly from RAG
    static const uint32_t regs[]  = {
        BASE_ADDR_VENDOR_DEVICE_ID,
        BASE_ADDR_STATUS_COMMAND,
        BASE_ADDR_CLASSCODE,
        BASE_ADDR_BAR0,
        BASE_ADDR_INT_PIN
    };

    static const uint32_t masks[] = {
        VENDOR_DEVICE_ID_ALL_MASK,
        STATUS_COMMAND_ALL_MASK,
        CLASSCODE_ALL_MASK,
        BAR0_ALL_MASK,
        INT_PIN_ALL_MASK
    };

    uint32_t def_fail_cnt = 0u;
    uint32_t wr_fail_cnt  = 0u;

    // Step 2: Read defaults (no golden defaults provided; capture for reference only)
    for (unsigned i = 0; i < (sizeof(regs)/sizeof(regs[0])); ++i)
    {
        volatile uint32_t addr = regs[i];
        (void)addr; // suppress unused warning if not used later
        // Default comparison to a provided table is not possible without golden values.
        // We do not increment def_fail_cnt due to lack of RAG-provided default values.
    }

    // Step 3/4: Write patterns and read back using field masks from RAG
    for (unsigned i = 0; i < (sizeof(regs)/sizeof(regs[0])); ++i)
    {
        volatile uint32_t addr = regs[i];
        uint32_t mask = masks[i];

        for (unsigned p = 0; p < num_patterns; ++p)
        {
            uint32_t wr = patterns[p] & mask; // only affect documented field bits
            write_reg(addr, wr);
            uint32_t rd = read_reg(addr) & mask;

            if (rd != wr)
            {
                wr_fail_cnt++;
            }
        }
    }

    // Step 5: Validate
    if ((def_fail_cnt != 0u) || (wr_fail_cnt != 0u))
    {
        while(1);
    }

    // Pass condition reached
}
