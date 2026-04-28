#include "test_gpio_nedge_alternate_pads_en.h"
#include <stdint.h>

#define EXTI_BASE  0x40013C00UL
#define EXTI_FTSR  (*(volatile uint32_t *)(EXTI_BASE + 0x0C))

static void print(const char *s)
{
    volatile uint32_t *ITM_TCR = (uint32_t *)0xE0000E80;
    volatile uint32_t *ITM_STIM = (uint32_t *)0xE0000000;
    if (*ITM_TCR & 1)
        while (*s) *ITM_STIM = *s++;
}

#define PASS()  do { print("GPIO NEDGE ALT PADS TEST PASSED\n"); } while (0)
#define FAIL(x) do { print(x); while (1); } while (0)

int main(void)
{
    int pins[] = {0, 2, 4, 6};

    for (int i = 0; i < 4; i++)
    {
        EXTI_FTSR |= (1 << pins[i]);
        if (!(EXTI_FTSR & (1 << pins[i])))
            FAIL("FAIL: NEDGE ALT PAD\n");
    }

    PASS();
    while (1);
}
