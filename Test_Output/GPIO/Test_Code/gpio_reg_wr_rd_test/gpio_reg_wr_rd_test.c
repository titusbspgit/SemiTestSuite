#include "gpio_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/*
 * Module: GPIO
 * Feature: GPIO output register read/write verification
 * Test: gpio_reg_wr_rd_test
 * Description: Enables GPIOA, configures PA0 as output, writes high and low to the output data register,
 *              and verifies the corresponding bit state by reading back.
 * Remarks: RCC_AHB1ENR must enable the GPIOA clock before accessing GPIOA_MODER and GPIOA_ODR.
 *          PA0 must be configured as output in GPIOA_MODER.
 * Impacted Registers: RCC_AHB1ENR, GPIOA_MODER, GPIOA_ODR
 * Validation: After setting PA0 high, GPIOA_ODR bit 0 must read as 1; after clearing PA0 low,
 *             GPIOA_ODR bit 0 must read as 0. On any mismatch, the test fails; otherwise, it passes.
 */

/* NOTE: Register addresses and bit positions are placeholders. Update with SoC-specific values. */
#define REG32(addr)                (*(volatile uint32_t*)(addr))
#define RCC_AHB1ENR                REG32(0x00000000u) /* TODO: set correct base address */
#define GPIOA_MODER                REG32(0x00000000u) /* TODO: set correct base address */
#define GPIOA_ODR                  REG32(0x00000000u) /* TODO: set correct base address */

/* Bit/mask definitions (verify with the device reference manual) */
#define BIT_GPIOA_EN               (1u << 0)          /* TODO: verify bit index for GPIOA clock enable */
#define MASK_PA0_MODE              (0x3u << 0)        /* MODE[1:0] for PA0 */
#define MODE_OUTPUT_PA0            (0x1u << 0)        /* 01b = output */
#define BIT_PA0_ODR                (1u << 0)          /* ODR bit for PA0 */

void gpio_reg_wr_rd_test(void)
{
    printf("[TRACE] Start: gpio_reg_wr_rd_test\n");

    /* Step 1: Enable GPIOA clock */
    printf("[TRACE] Step 1: Enable GPIOA clock\n");
    RCC_AHB1ENR |= BIT_GPIOA_EN;

    /* Step 2: Configure PA0 as output */
    printf("[TRACE] Step 2: Configure PA0 as output\n");
    uint32_t moder = GPIOA_MODER;
    moder &= ~MASK_PA0_MODE;     /* clear mode bits */
    moder |= MODE_OUTPUT_PA0;    /* set to output */
    GPIOA_MODER = moder;

    /* Step 3: Drive PA0 high */
    printf("[TRACE] Step 3: Drive PA0 high\n");
    GPIOA_ODR |= BIT_PA0_ODR;

    /* Step 4: Read back high */
    printf("[TRACE] Step 4: Verify PA0 reads back HIGH\n");
    uint32_t odr = GPIOA_ODR;
    if (((odr & BIT_PA0_ODR) ? 1u : 0u) != 1u)
    {
        printf("[TRACE] FAIL: Expected ODR bit0 = 1 after set, got %u\n", (unsigned)((odr & BIT_PA0_ODR) ? 1u : 0u));
        while(1); /* Fail condition */
    }

    /* Step 5: Drive PA0 low */
    printf("[TRACE] Step 5: Drive PA0 low\n");
    GPIOA_ODR &= ~BIT_PA0_ODR;

    /* Step 6: Read back low */
    printf("[TRACE] Step 6: Verify PA0 reads back LOW\n");
    odr = GPIOA_ODR;
    if (((odr & BIT_PA0_ODR) ? 1u : 0u) != 0u)
    {
        printf("[TRACE] FAIL: Expected ODR bit0 = 0 after clear, got %u\n", (unsigned)((odr & BIT_PA0_ODR) ? 1u : 0u));
        while(1); /* Fail condition */
    }

    printf("[TRACE] PASS: gpio_reg_wr_rd_test\n");
}
