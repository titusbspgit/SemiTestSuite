#include "test_gpio_negedge_intr_en.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */

// RAG data not available, using placeholder base/offset values
#define BASE_ADDR_GPIO0                 0x0      // TODO: Update with GPIO0 base address
#define BASE_ADDR_GPIO0_IOCTRL          0x0      // TODO: Update with IOCTRL base address
#define GPIO_INTR_RAW_STCLR1_OFFSET     0x0      // TODO: Update
#define GP0_INTR1_INTR_EN1_OFFSET       0x0      // TODO: Update
#define GP0_INTR1_INTR_STS1_OFFSET      0x0      // TODO: Update

// Given in Excel plan: pad driver address used to synthesize edges
#define PAD_DRIVER_ADDR                 0xA0243FFC  // As provided in test plan

// Per-pin IO control addressing
#define GP0_GPIO_PIN_STRIDE             0x4      // TODO: Confirm stride
static inline uint32_t gp0_gpio_pin_addr(uint32_t n)
{
    return (uint32_t)(BASE_ADDR_GPIO0_IOCTRL + (n * GP0_GPIO_PIN_STRIDE));
}

// Field bits (placeholders)
#define GPIO_CTRL_INPUT_MODE_BIT        20       // TODO: Update (configure input mode/doe)
#define GPIO_CTRL_NEGEDGE_EN_BIT        18       // TODO: Update
#define GPIO_CTRL_RAW_CLR_BIT           16       // TODO: Update (per-pin raw clear)

/* Low-level access */
static inline void write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

static inline void wait_on(unsigned count)
{
    volatile unsigned i = count * 100; // coarse placeholder delay
    while (i--) { __asm__ __volatile__("nop"); }
}

/* Test Function */
void test_gpio_negedge_intr_en(void)
{
    uint32_t test_err = 0;

    printf("Running %s\n", "test_gpio_negedge_intr_en");

    // Step 1: Initialize platform interrupt source and pad driver
    // NOTE: System-level interrupt enables/clears are SoC-specific and omitted here.
    write_reg(PAD_DRIVER_ADDR, 0xFFFFFFFFu); // drive all pads high

    // Step 2: Configure GPIOs 8..39 for input + negedge + clear raw
    for (uint32_t i = 0; i < 32; ++i)
    {
        uint32_t pin = 8u + i;
        uint32_t addr1 = gp0_gpio_pin_addr(pin);
        uint32_t cfg = (1u << GPIO_CTRL_INPUT_MODE_BIT) | (1u << GPIO_CTRL_NEGEDGE_EN_BIT) | (1u << GPIO_CTRL_RAW_CLR_BIT);
        write_reg(addr1, cfg);
        wait_on(10);
    }

    // Step 3: For each bit position 0..31 (pins 8..39)
    for (uint32_t i = 0; i < 32; ++i)
    {
        uint32_t wr_val = 1u << i;

        // a) Pre-clear group raw status
        write_reg(BASE_ADDR_GPIO0 + GPIO_INTR_RAW_STCLR1_OFFSET, wr_val);

        // b) Enable only current bit in group enable
        write_reg(BASE_ADDR_GPIO0 + GP0_INTR1_INTR_EN1_OFFSET, wr_val);
        wait_on(10);

        // c) Arm waiter flag and generate a falling edge on only this bit
        // Drive all-high, then drive this bit low
        write_reg(PAD_DRIVER_ADDR, 0xFFFFFFFFu);
        wait_on(30);
        write_reg(PAD_DRIVER_ADDR, ~wr_val);

        // d) Wait for the interrupt with a timeout
        uint32_t timeout = 5000u;
        int int_pend = 1; // armed
        while (int_pend && timeout--)
        {
            // Poll masked group status for the current bit
            uint32_t sts = read_reg(BASE_ADDR_GPIO0 + GP0_INTR1_INTR_STS1_OFFSET);
            if (sts & wr_val) { int_pend = 0; }
            wait_on(10);
        }
        if (timeout == 0)
        {
            printf("Timeout waiting for negedge interrupt on bit %lu\n", (unsigned long)i);
            test_err++;
            // Continue testing remaining bits
        }
        else
        {
            // Step 4: On interrupt, service conditions
            // Read per-pin control (expect input state low - device specific; placeholder check not enforced)
            uint32_t rdata = read_reg(gp0_gpio_pin_addr(8u + i));
            (void)rdata; // NOTE: Without real field map for DIN, skip evaluation here.

            // Confirm group status bit is set
            uint32_t rdata_grp = read_reg(BASE_ADDR_GPIO0 + GP0_INTR1_INTR_STS1_OFFSET);
            if ((rdata_grp & wr_val) == 0)
            {
                printf("Group STS not set for bit %lu\n", (unsigned long)i);
                test_err++;
            }

            // Clear per-pin raw (keep input mode) and clear group raw
            write_reg(gp0_gpio_pin_addr(8u + i), (1u << GPIO_CTRL_INPUT_MODE_BIT) | (1u << GPIO_CTRL_RAW_CLR_BIT));
            write_reg(BASE_ADDR_GPIO0 + GPIO_INTR_RAW_STCLR1_OFFSET, wr_val);

            // Verify group status reads zero after clears
            rdata_grp = read_reg(BASE_ADDR_GPIO0 + GP0_INTR1_INTR_STS1_OFFSET);
            if (rdata_grp != 0x0u)
            {
                printf("Group STS not cleared after service for bit %lu (0x%08lX)\n", (unsigned long)i, (unsigned long)rdata_grp);
                test_err++;
            }

            // Restore pads high for next iteration
            write_reg(PAD_DRIVER_ADDR, 0xFFFFFFFFu);
        }
    }

    // Step 5: Completion
    if (test_err != 0)
    {
        printf("Test Failed\n");
        while(1);
    }

    printf("Test Passed\n");
}
