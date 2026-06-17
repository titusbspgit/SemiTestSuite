#include "gpio_reg_wr_rd_test.h"
#include <stdint.h>
#include <stdio.h>

/* Register Definitions */

// RAG data not available, using placeholder base/offset values
#define BASE_ADDR_GPIO0                 0x0      // TODO: Update with GPIO0 base address
#define BASE_ADDR_GPIO0_IOCTRL          0x0      // TODO: Update with IOCTRL base address
#define GPIO_IO_CTRL_GROUP1_OFFSET      0x0      // TODO: Update
#define GPIO_IO_CTRL_GROUP2_OFFSET      0x0      // TODO: Update
#define GPIO_IO_CTRL_GROUP3_OFFSET      0x0      // TODO: Update
#define GPIO_IO_CTRL_GROUP4_OFFSET      0x0      // TODO: Update
#define GPIO_DOUT_GROUP1_OFFSET         0x0      // TODO: Update
#define GPIO_DOUT_GROUP2_OFFSET         0x0      // TODO: Update
#define GPIO_DOUT_GROUP3_OFFSET         0x0      // TODO: Update
#define GPIO_DOUT_GROUP4_OFFSET         0x0      // TODO: Update
#define GPIO_DIN_GROUP1_OFFSET          0x0      // TODO: Update
#define GPIO_DIN_GROUP2_OFFSET          0x0      // TODO: Update
#define GPIO_DIN_GROUP3_OFFSET          0x0      // TODO: Update
#define GPIO_DIN_GROUP4_OFFSET          0x0      // TODO: Update
#define GPIO_INTR_RAW_STCLR1_OFFSET     0x0      // TODO: Update
#define GP0_INTR1_INTR_EN1_OFFSET       0x0      // TODO: Update
#define GP0_INTR1_INTR_STS1_OFFSET      0x0      // TODO: Update
#define GP0_INTR2_INTR_EN1_OFFSET       0x0      // TODO: Update
#define GP0_INTR2_INTR_STS1_OFFSET      0x0      // TODO: Update

// Per-pin IO control addressing
#define GP0_GPIO_PIN_STRIDE             0x4      // TODO: Confirm stride
static inline uint32_t gp0_gpio_pin_addr(uint32_t n)
{
    // Address of per-pin IOCTRL register for pin n (8..39)
    return (uint32_t)(BASE_ADDR_GPIO0_IOCTRL + (n * GP0_GPIO_PIN_STRIDE));
}

// Field bits (placeholders)
#define GPIO_CTRL_DOE_BIT               20       // TODO: Update (direction/output enable or data out enable)
#define GPIO_CTRL_NEGEDGE_EN_BIT        18       // TODO: Update
#define GPIO_CTRL_POSEDGE_EN_BIT        17       // TODO: Update
#define GPIO_CTRL_RAW_CLR_BIT           16       // TODO: Update (write 1 to clear per-pin raw)

/* Low-level access */
static inline void write_reg(uint32_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t read_reg(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

/* Helpers */
static inline uint32_t group_offset_for_pin(uint32_t pin, uint32_t off_g1, uint32_t off_g2, uint32_t off_g3, uint32_t off_g4)
{
    if (pin < 32) return off_g1;        // pins 0..31
    if (pin < 64) return off_g2;        // pins 32..63
    if (pin < 96) return off_g3;        // pins 64..95
    return off_g4;                      // pins 96..127
}

static inline uint32_t group_bit_for_pin(uint32_t pin)
{
    return (pin & 31u);
}

/* Test Function */
void gpio_reg_wr_rd_test(void)
{
    uint32_t def_fail_cnt = 0;
    uint32_t wr_fail_cnt = 0;

    printf("Running %s\n", "gpio_reg_wr_rd_test");

    // Step 1: Based on Excel steps - Default value checks for selected registers
    // Registers include per-pin GP0_GPIO_8..GP0_GPIO_39 and grouped GPIO/INTR registers.
    // RAG data not available, using placeholder default/mask values.

    // Default check across pins 8..39
    for (uint32_t pin = 8; pin <= 39; ++pin)
    {
        uint32_t addr = gp0_gpio_pin_addr(pin);
        // Example default/masks (placeholders)
        uint32_t read_mask = 0xFFFFFFFF;   // TODO: Replace per-register mask
        uint32_t def_val   = 0x00000000;   // TODO: Replace with documented default

        if (read_mask == 0x0) continue;    // Skip if no readable bits

        uint32_t data_rd = read_reg(addr);
        uint32_t data = (data_rd & 0xFFFFFFFEu); // Clear LSB as per test plan
        if (data != def_val)
        {
            def_fail_cnt++;
            printf("Default mismatch: pin %lu addr=0x%08lX rd=0x%08lX exp=0x%08lX\n",
                   (unsigned long)pin, (unsigned long)addr, (unsigned long)data, (unsigned long)def_val);
        }
    }

    // Default check for group/interrupt registers (illustrative, using placeholders)
    {
        uint32_t regs[] = {
            BASE_ADDR_GPIO0 + GPIO_INTR_RAW_STCLR1_OFFSET,
            BASE_ADDR_GPIO0 + GP0_INTR1_INTR_EN1_OFFSET,
            BASE_ADDR_GPIO0 + GP0_INTR1_INTR_STS1_OFFSET,
            BASE_ADDR_GPIO0 + GP0_INTR2_INTR_EN1_OFFSET,
            BASE_ADDR_GPIO0 + GP0_INTR2_INTR_STS1_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_IO_CTRL_GROUP1_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_IO_CTRL_GROUP2_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_IO_CTRL_GROUP3_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_IO_CTRL_GROUP4_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_DOUT_GROUP1_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_DOUT_GROUP2_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_DOUT_GROUP3_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_DOUT_GROUP4_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_DIN_GROUP1_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_DIN_GROUP2_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_DIN_GROUP3_OFFSET,
            BASE_ADDR_GPIO0 + GPIO_DIN_GROUP4_OFFSET
        };
        for (unsigned i = 0; i < sizeof(regs)/sizeof(regs[0]); ++i)
        {
            uint32_t addr = regs[i];
            uint32_t read_mask = 0xFFFFFFFF; // TODO: Replace
            uint32_t def_val   = 0x00000000; // TODO: Replace per register
            if (read_mask == 0) continue;
            uint32_t data = (read_reg(addr) & 0xFFFFFFFEu);
            if (data != def_val)
            {
                def_fail_cnt++;
                printf("Default mismatch: reg[%u] addr=0x%08lX rd=0x%08lX exp=0x%08lX\n",
                       i, (unsigned long)addr, (unsigned long)data, (unsigned long)def_val);
            }
        }
    }

    // Step 2: Masked write-read verification across six patterns
    const uint32_t chk_val[6] = { 0xFFFFFFFFu, 0xAAAAAAA Au, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u };
    for (unsigned p = 0; p < 6; ++p)
    {
        uint32_t data_wr = chk_val[p];

        // Write phase: per-pin
        for (uint32_t pin = 8; pin <= 39; ++pin)
        {
            uint32_t addr = gp0_gpio_pin_addr(pin);
            uint32_t write_mask = 0x00000000u; // TODO: Replace per register write mask
            if (write_mask == 0x0) continue;
            write_reg(addr, (data_wr & write_mask));
        }

        // Read/compare phase: per-pin
        for (uint32_t pin = 8; pin <= 39; ++pin)
        {
            uint32_t addr = gp0_gpio_pin_addr(pin);
            uint32_t write_mask = 0x00000000u; // TODO
            uint32_t read_mask  = 0x00000000u; // TODO
            uint32_t def_val    = 0x00000000u; // TODO
            if (write_mask == 0x0 || read_mask == 0x0) continue;
            uint32_t data_rd = (read_reg(addr) & read_mask);
            uint32_t wr_n = (write_mask ^ 0xFFFFFFFFu);
            uint32_t exp_val = ((data_wr & read_mask & write_mask) | (wr_n & read_mask & def_val));
            if (data_rd != exp_val)
            {
                wr_fail_cnt++;
                printf("WR mismatch: pin %lu addr=0x%08lX rd=0x%08lX exp=0x%08lX pat=0x%08lX\n",
                       (unsigned long)pin, (unsigned long)addr, (unsigned long)data_rd, (unsigned long)exp_val, (unsigned long)data_wr);
            }
        }
    }

    // Step 3: Completion
    if (def_fail_cnt > 0 || wr_fail_cnt > 0)
    {
        printf("Test Failed\n");
        while(1);
    }

    printf("Test Passed\n");
}
