#include "gpio_reg_wr_rd_test.h"
#include <stdint.h>

// Impacted Registers (from test plan):
// - RCC_AHB1ENR
// - GPIOA_MODER
// - GPIOA_ODR
// NOTE: Addresses and some bit fields are placeholders. Replace TODOs with SoC-specific values.

#define REG_RCC_AHB1ENR (*(volatile uint32_t*)0xDEAD0000) // TODO: set correct address for RCC_AHB1ENR
#define REG_GPIOA_MODER (*(volatile uint32_t*)0xDEAD0004) // TODO: set correct address for GPIOA_MODER
#define REG_GPIOA_ODR   (*(volatile uint32_t*)0xDEAD0008) // TODO: set correct address for GPIOA_ODR

// Bit/mask placeholders (replace with actual values for your device)
#define RCC_AHB1ENR_GPIOA_EN_MASK   (0x00000001u) // TODO: correct bit for GPIOA clock enable
#define GPIOA_MODER_PA0_MASK        (0x00000003u) // TODO: correct mask for PA0 mode bits
#define GPIOA_MODER_PA0_OUTPUT      (0x00000001u) // TODO: correct value to set PA0 as output mode
#define GPIOA_ODR_BIT0_MASK         (1u << 0)     // Validation specifies bit 0

void gpio_reg_wr_rd_test(void)
{
    // Step 1: Enable clocks
    // 1) Enable GPIOA clock.
    REG_RCC_AHB1ENR |= RCC_AHB1ENR_GPIOA_EN_MASK; // TODO: ensure write enables GPIOA clock

    // Step 2: Configure registers
    // 2) Configure PA0 as output.
    uint32_t moder = REG_GPIOA_MODER;
    moder &= ~GPIOA_MODER_PA0_MASK;       // clear PA0 mode bits
    moder |= GPIOA_MODER_PA0_OUTPUT;      // set PA0 to output mode
    REG_GPIOA_MODER = moder;

    // Step 3: Perform write
    // 3) Drive PA0 high.
    REG_GPIOA_ODR |= GPIOA_ODR_BIT0_MASK; // set bit 0 high

    // Step 4: Read back
    // 4) Verify PA0 output bit reads back high.
    uint32_t odr_read = REG_GPIOA_ODR;

    // Step 5: Validate
    // Validation/Acceptance Criteria (from test plan):
    // - After setting PA0 high, GPIOA_ODR bit 0 must read as 1.
    // - After clearing PA0 low, GPIOA_ODR bit 0 must read as 0.
    if ((odr_read & GPIOA_ODR_BIT0_MASK) == 0u)
    {
        // Fail condition: expected bit 0 to be 1 after setting high
        while (1) { }
    }

    // Continue test sequence per procedure:
    // 5) Drive PA0 low.
    REG_GPIOA_ODR &= ~GPIOA_ODR_BIT0_MASK; // clear bit 0 low

    // 6) Verify PA0 output bit reads back low.
    odr_read = REG_GPIOA_ODR;
    if ((odr_read & GPIOA_ODR_BIT0_MASK) != 0u)
    {
        // Fail condition: expected bit 0 to be 0 after clearing low
        while (1) { }
    }

    // 7) Declare test pass.
    // Pass condition: reached without entering fail loops
}
