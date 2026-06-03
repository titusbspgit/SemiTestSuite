#ifndef MOCK_MMIO_H
#define MOCK_MMIO_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

void mmio_init(void);
void mmio_write32(uint32_t addr, uint32_t val);
uint32_t mmio_read32(uint32_t addr);
void mmio_write16(uint32_t addr, uint16_t val);
uint16_t mmio_read16(uint32_t addr);
void mmio_write8(uint32_t addr, uint8_t val);
uint8_t mmio_read8(uint32_t addr);
void mmio_write32_masked(uint32_t addr, uint32_t val, uint32_t mask);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_MMIO_H */
