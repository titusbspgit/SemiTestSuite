#include "gpio_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/*
 * Test Case: gpio_reg_wr_rd_test
 * Module: GPIO
 * Feature: GPIO output register read/write verification
 * Description: Enables GPIOA, configures PA0 as output, writes high and low to
 *              the output data register, and verifies the corresponding bit state
 *              by reading back.
 * Impacted Registers: RCC_AHB1ENR, GPIOA_MODER, GPIOA_ODR
 * Validation: After setting PA0 high, GPIOA_ODR bit 0 must read as 1; after
 *             clearing PA0 low, GPIOA_ODR bit 0 must read as 0. On any mismatch,
 *             the test fails; otherwise, it passes.
 * Remarks: RCC_AHB1ENR must enable the GPIOA clock before accessing GPIOA_MODER
 *          and GPIOA_ODR. PA0 must be configured as output in GPIOA_MODER.
 */

#define REG32(addr) (*(volatile uint32_t *)(addr))

/* TODO: Replace base addresses and offsets with SoC-specific values */
#define RCC_BASE                0x40023800u /* placeholder */
#define RCC_AHB1ENR_ADDR        (RCC_BASE + 0x30u)

#define GPIOA_BASE              0x40020000u /* placeholder */
#define GPIO_MODER_ADDR(port)   ((port) + 0x00u)
#define GPIO_ODR_ADDR(port)     ((port) + 0x14u)

/* Bit fields */
#define RCC_AHB1ENR_GPIOAEN     (1u << 0)
#define GPIO_MODER_MODE0_Pos    0u
#define GPIO_MODER_MODE0_Msk    (0x3u << GPIO_MODER_MODE0_Pos)
#define GPIO_MODER_MODE0_OUTPUT (0x1u << GPIO_MODER_MODE0_Pos)

void gpio_reg_wr_rd_test(void)
{
    printf("[gpio_reg_wr_rd_test] START\n");

    /* Step 1: Enable GPIOA clock */
    REG32(RCC_AHB1ENR_ADDR) |= RCC_AHB1ENR_GPIOAEN;
    (void)REG32(RCC_AHB1ENR_ADDR); /* dummy read to ensure write completes */
    printf(" - Clock enabled: RCC_AHB1ENR |= GPIOAEN\n");

    /* Step 2: Configure PA0 as output */
    uint32_t moder = REG32(GPIO_MODER_ADDR(GPIOA_BASE));
    moder &= ~GPIO_MODER_MODE0_Msk;            /* clear mode bits for PA0 */
    moder |= GPIO_MODER_MODE0_OUTPUT;          /* set PA0 to output mode */
    REG32(GPIO_MODER_ADDR(GPIOA_BASE)) = moder;
    printf(" - Configured PA0 as output (GPIOA_MODER)\n");

    /* Step 3: Drive PA0 high */
    REG32(GPIO_ODR_ADDR(GPIOA_BASE)) |= (1u << 0);
    uint32_t odr = REG32(GPIO_ODR_ADDR(GPIOA_BASE));
    printf(" - Drive PA0 HIGH; GPIOA_ODR=0x%08lx\n", (unsigned long)odr);

    /* Step 4: Validate HIGH */
    if ((odr & (1u << 0)) == 0u)
    {
        printf("[gpio_reg_wr_rd_test] FAIL: Expected PA0 HIGH, read LOW\n");
        while (1) { /* fail-stop */ }
    }

    /* Step 5: Drive PA0 low */
    REG32(GPIO_ODR_ADDR(GPIOA_BASE)) &= ~(1u << 0);
    odr = REG32(GPIO_ODR_ADDR(GPIOA_BASE));
    printf(" - Drive PA0 LOW; GPIOA_ODR=0x%08lx\n", (unsigned long)odr);

    /* Step 6: Validate LOW */
    if ((odr & (1u << 0)) != 0u)
    {
        printf("[gpio_reg_wr_rd_test] FAIL: Expected PA0 LOW, read HIGH\n");
        while (1) { /* fail-stop */ }
    }

    printf("[gpio_reg_wr_rd_test] PASS\n");
}
