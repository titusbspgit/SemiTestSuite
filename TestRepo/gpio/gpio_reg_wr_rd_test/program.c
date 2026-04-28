#include "gpio_reg_wr_rd_test.h"
#include <stdint.h>

#define RCC_BASE        0x40023800UL
#define GPIOA_BASE      0x40020000UL

#define RCC_AHB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define GPIOA_MODER     (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

static void print(const char *s)
{
    volatile uint32_t *ITM_TCR = (uint32_t *)0xE0000E80;
    volatile uint32_t *ITM_STIM = (uint32_t *)0xE0000000;
    if (*ITM_TCR & 1)
        while (*s) *ITM_STIM = *s++;
}

#define PASS()  do { print("GPIO REG WR/RD TEST PASSED\n"); } while (0)
#define FAIL(x) do { print(x); while (1); } while (0)

int main(void)
{
    RCC_AHB1ENR |= (1 << 0);

    GPIOA_MODER &= ~(3U << 0);
    GPIOA_MODER |=  (1U << 0);    // PA0 output

    GPIOA_ODR |= (1 << 0);
    if (!(GPIOA_ODR & (1 << 0)))
        FAIL("FAIL: GPIO WRITE HIGH\n");

    GPIOA_ODR &= ~(1 << 0);
    if (GPIOA_ODR & (1 << 0))
        FAIL("FAIL: GPIO WRITE LOW\n");

    PASS();
    while (1);
}
