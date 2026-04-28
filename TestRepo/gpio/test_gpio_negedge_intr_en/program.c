#include "test_gpio_negedge_intr_en.h"
#include <stdint.h>

#define RCC_BASE     0x40023800UL
#define GPIOA_BASE   0x40020000UL
#define SYSCFG_BASE  0x40013800UL
#define EXTI_BASE    0x40013C00UL

#define RCC_AHB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB2ENR  (*(volatile uint32_t *)(RCC_BASE + 0x44))
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define SYSCFG_EXTICR1 (*(volatile uint32_t *)(SYSCFG_BASE + 0x08))
#define EXTI_IMR     (*(volatile uint32_t *)(EXTI_BASE + 0x00))
#define EXTI_FTSR    (*(volatile uint32_t *)(EXTI_BASE + 0x0C))

static void print(const char *s)
{
    volatile uint32_t *ITM_TCR = (uint32_t *)0xE0000E80;
    volatile uint32_t *ITM_STIM = (uint32_t *)0xE0000000;
    if (*ITM_TCR & 1)
        while (*s) *ITM_STIM = *s++;
}

#define PASS()  do { print("GPIO NEG EDGE INTR TEST PASSED\n"); } while (0)
#define FAIL(x) do { print(x); while (1); } while (0)

int main(void)
{
    RCC_AHB1ENR |= (1 << 0);
    RCC_APB2ENR |= (1 << 14);

    GPIOA_MODER &= ~(3U << (1 * 2));   // PA1 input
    SYSCFG_EXTICR1 &= ~(0xF << 4);     // PA1 → EXTI1

    EXTI_IMR  |= (1 << 1);
    EXTI_FTSR |= (1 << 1);

    if (!(EXTI_FTSR & (1 << 1)))
        FAIL("FAIL: NEG EDGE NOT ENABLED\n");

    PASS();
    while (1);
}
