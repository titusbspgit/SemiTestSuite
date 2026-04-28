#include "test_gpio_pedge_alternate_pads_en.h"
#include <stdint.h>

#define EXTI_BASE  0x40013C00UL
#define EXTI_RTSR  (*(volatile uint32_t *)(EXTI_BASE + 0x08))

static void print(const char *s)
{
    volatile uint32_t *ITM_TCR = (uint32_t *)0xE0000E80;
    volatile uint32_t *ITM_STIM = (uint32_t *)0xE0000000;
    if (*ITM_TCR & 1)
        while (*s) *ITM_STIM = *s++;
}

#define PASS()  do { print("GPIO PEDGE ALT PADS TEST PASSED\n"); } while (0)
#define FAIL(x) do { print(x); while (1); } while (0)

int main(void)
{
    int pins[] = {0, 2, 4, 6};

    for (int i = 0; i < 4; i++)
    {
        EXTI_RTSR |= (1 << pins[i]);
        if (!(EXTI_RTSR & (1 << pins[i])))
            FAIL("FAIL: PEDGE ALT PAD\n");
    }

    PASS();
    while (1);
}
