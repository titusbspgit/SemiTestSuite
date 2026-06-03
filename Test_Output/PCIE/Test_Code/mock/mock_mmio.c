#include "mock_mmio.h"
#include <string.h>

#define MMIO_MAP_CAPACITY 1024

typedef struct { uint32_t addr; uint32_t val; uint8_t used; } mmio_entry_t;
static mmio_entry_t g_map[MMIO_MAP_CAPACITY];

static mmio_entry_t* find_slot(uint32_t addr)
{
    for (unsigned i = 0; i < MMIO_MAP_CAPACITY; ++i) {
        if (g_map[i].used && g_map[i].addr == addr) return &g_map[i];
    }
    for (unsigned i = 0; i < MMIO_MAP_CAPACITY; ++i) {
        if (!g_map[i].used) { g_map[i].used = 1; g_map[i].addr = addr; g_map[i].val = 0; return &g_map[i]; }
    }
    return 0; /* map full - simplistic */
}

void mmio_init(void)
{
    memset(g_map, 0, sizeof(g_map));
}

void mmio_write32(uint32_t addr, uint32_t val)
{
    mmio_entry_t* e = find_slot(addr);
    if (e) e->val = val;
}

uint32_t mmio_read32(uint32_t addr)
{
    mmio_entry_t* e = find_slot(addr);
    return e ? e->val : 0u;
}

void mmio_write16(uint32_t addr, uint16_t val)
{
    uint32_t full = mmio_read32(addr & ~0x3u);
    uint8_t shift = (addr & 0x2u) ? 16 : 0;
    full &= ~(0xFFFFu << shift);
    full |= ((uint32_t)val) << shift;
    mmio_write32(addr & ~0x3u, full);
}

uint16_t mmio_read16(uint32_t addr)
{
    uint32_t full = mmio_read32(addr & ~0x3u);
    uint8_t shift = (addr & 0x2u) ? 16 : 0;
    return (uint16_t)((full >> shift) & 0xFFFFu);
}

void mmio_write8(uint32_t addr, uint8_t val)
{
    uint32_t full = mmio_read32(addr & ~0x3u);
    uint8_t shift = (addr & 0x3u) * 8;
    full &= ~(0xFFu << shift);
    full |= ((uint32_t)val) << shift;
    mmio_write32(addr & ~0x3u, full);
}

uint8_t mmio_read8(uint32_t addr)
{
    uint32_t full = mmio_read32(addr & ~0x3u);
    uint8_t shift = (addr & 0x3u) * 8;
    return (uint8_t)((full >> shift) & 0xFFu);
}

void mmio_write32_masked(uint32_t addr, uint32_t val, uint32_t mask)
{
    uint32_t cur = mmio_read32(addr);
    cur = (cur & ~mask) | (val & mask);
    mmio_write32(addr, cur);
}
