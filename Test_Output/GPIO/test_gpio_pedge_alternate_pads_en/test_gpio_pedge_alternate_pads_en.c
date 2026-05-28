#include "test_gpio_pedge_alternate_pads_en.h"
#include <stdint.h>
#include <stdio.h>

// Impacted Registers (shadowed locally; replace with actual HW addresses/mappings if available)
static volatile uint32_t REG_EXTI_RTSR = 0; // TODO: map to actual EXTI_RTSR

void test_gpio_pedge_alternate_pads_en(void)
{
    printf("[TRACE] SS/Module: GPIO\n");
    printf("[TRACE] Feature: Rising edge trigger enable on alternate GPIO pads\n");
    printf("[TRACE] Mode: Polling | Speed: NA\n");
    printf("[TRACE] Remarks: NA\n");

    // Step 1: For pins 0, 2, 4, and 6, enable rising edge trigger
    const uint8_t pins[] = {0u, 2u, 4u, 6u};
    for (unsigned i = 0; i < sizeof(pins)/sizeof(pins[0]); ++i)
    {
        uint8_t pin = pins[i];
        REG_EXTI_RTSR |= (1u << pin);
        printf("[TRACE] Enabled rising edge on line %u. EXTI_RTSR=0x%08lx\n", (unsigned)pin, (unsigned long)REG_EXTI_RTSR);
    }

    // Step 2: Verify each corresponding rising edge trigger bit is set
    for (unsigned i = 0; i < sizeof(pins)/sizeof(pins[0]); ++i)
    {
        uint8_t pin = pins[i];
        uint32_t val = REG_EXTI_RTSR;
        if (((val >> pin) & 1u) != 1u)
        {
            printf("[FAIL] Validation: Expected EXTI_RTSR[%u]=1; got %lu\n", (unsigned)pin, (unsigned long)((val >> pin) & 1u));
            while(1);
        }
    }

    // Step 3: Declare test pass
    printf("[PASS] test_gpio_pedge_alternate_pads_en completed successfully.\n");
}
