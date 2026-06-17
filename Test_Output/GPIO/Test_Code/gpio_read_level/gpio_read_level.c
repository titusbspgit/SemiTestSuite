#include "gpio_read_level.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */

// RAG data not available, using placeholder
#define BASE_ADDR_GPIO   0x0  // TODO: Update
#define GPIO_OFFSET      0x0  // TODO: Update
#define GPIO_DIR         0x0  // TODO: Update

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

void gpio_read_level(void)
{
    uint32_t val;

    printf("Running %s\n", "gpio_read_level");

    // Step 1: Based on Excel steps

    // Step 2: Register configuration
    write_reg(BASE_ADDR_GPIO, 0x1);  // Example / TODO

    // Step 3: Read
    val = read_reg(BASE_ADDR_GPIO);

    // Step 4: Validation
    if (val == 0)
    {
        printf("Test Failed\n");
        while(1);
    }

    printf("Test Passed\n");
}
