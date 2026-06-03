#ifndef PCIE_ADDR_MAP_H
#define PCIE_ADDR_MAP_H

/*
 Device-agnostic PCIe address map for tests.
 - Override by defining PCIE_ADDR_MAP_PROVIDED and supplying real values via your build system
   (e.g., -DPCIE_ADDR_MAP_PROVIDED and ensure this header is replaced or augmented).
 - The default values below are placeholders to allow build and mock-based execution.
*/

/* When a real map is not provided, use placeholder bases. */
#ifndef PCIE_ADDR_MAP_PROVIDED
#define PCIE_CFG_BASE     0xDEAD0000u  /* TODO: replace with real PCIe configuration space base */
#define PCIE_DBI_DSP_BASE 0xDEAD1000u  /* TODO: replace with real DBI DSP base */
#define PCIE_DBI_USP_BASE 0xDEAD2000u  /* TODO: replace with real DBI USP base */
#define PCIE_SII_RC_BASE  0xDEAD3000u  /* TODO: replace with real SII RC base */

/* Offsets for PCIE Config Registers (placeholders, sequential 4-byte) */
#define VENDOR_DEVICE_ID_OFFSET 0x000u /* TODO */
#define STATUS_COMMAND_OFFSET   0x004u /* TODO */
#define CLASSCODE_OFFSET        0x008u /* TODO */
#define BAR0_OFFSET             0x00Cu /* TODO */
#define INT_PIN_OFFSET          0x010u /* TODO */

/* Offsets for PCIE DBI DSP Registers (placeholders) */
#define CMD_STATUS_OFFSET 0x000u /* TODO */
#define CFG_SETUP_OFFSET  0x004u /* TODO */
#define OB_SIZE_OFFSET    0x008u /* TODO */
#define IRQ_EOI_OFFSET    0x00Cu /* TODO */

/* Offsets for PCIE DBI USP GPR Registers (placeholders) */
#define GPR0_OFFSET 0x000u /* TODO */
#define GPR1_OFFSET 0x004u /* TODO */
#define GPR2_OFFSET 0x008u /* TODO */

/* Offsets for PCIE SII RC Registers (placeholders) */
#define IB_BAR0_OFFSET    0x000u /* TODO */
#define IB_START_LO_OFFSET 0x004u /* TODO */
#define IB_OFFSET_OFFSET   0x008u /* TODO */

/* Absolute addresses (computed) */
#define PCIE_CFG_VENDOR_DEVICE_ID_ADDR  (PCIE_CFG_BASE + VENDOR_DEVICE_ID_OFFSET)
#define PCIE_CFG_STATUS_COMMAND_ADDR    (PCIE_CFG_BASE + STATUS_COMMAND_OFFSET)
#define PCIE_CFG_CLASSCODE_ADDR         (PCIE_CFG_BASE + CLASSCODE_OFFSET)
#define PCIE_CFG_BAR0_ADDR              (PCIE_CFG_BASE + BAR0_OFFSET)
#define PCIE_CFG_INT_PIN_ADDR           (PCIE_CFG_BASE + INT_PIN_OFFSET)

#define PCIE_DBI_DSP_CMD_STATUS_ADDR    (PCIE_DBI_DSP_BASE + CMD_STATUS_OFFSET)
#define PCIE_DBI_DSP_CFG_SETUP_ADDR     (PCIE_DBI_DSP_BASE + CFG_SETUP_OFFSET)
#define PCIE_DBI_DSP_OB_SIZE_ADDR       (PCIE_DBI_DSP_BASE + OB_SIZE_OFFSET)
#define PCIE_DBI_DSP_IRQ_EOI_ADDR       (PCIE_DBI_DSP_BASE + IRQ_EOI_OFFSET)

#define PCIE_DBI_USP_GPR0_ADDR          (PCIE_DBI_USP_BASE + GPR0_OFFSET)
#define PCIE_DBI_USP_GPR1_ADDR          (PCIE_DBI_USP_BASE + GPR1_OFFSET)
#define PCIE_DBI_USP_GPR2_ADDR          (PCIE_DBI_USP_BASE + GPR2_OFFSET)

#define PCIE_SII_RC_IB_BAR0_ADDR        (PCIE_SII_RC_BASE + IB_BAR0_OFFSET)
#define PCIE_SII_RC_IB_START_LO_ADDR    (PCIE_SII_RC_BASE + IB_START_LO_OFFSET)
#define PCIE_SII_RC_IB_OFFSET_ADDR      (PCIE_SII_RC_BASE + IB_OFFSET_OFFSET)

/* Default masks (placeholders). Replace with device-specific RO/RW masks when available. */
#define VENDOR_DEVICE_ID_RD_MASK 0xFFFFFFFFu
#define VENDOR_DEVICE_ID_WR_MASK 0xFFFFFFFFu
#define STATUS_COMMAND_RD_MASK   0xFFFFFFFFu
#define STATUS_COMMAND_WR_MASK   0xFFFFFFFFu
#define CLASSCODE_RD_MASK        0xFFFFFFFFu
#define CLASSCODE_WR_MASK        0xFFFFFFFFu
#define BAR0_RD_MASK             0xFFFFFFFFu
#define BAR0_WR_MASK             0xFFFFFFFFu
#define INT_PIN_RD_MASK          0xFFFFFFFFu
#define INT_PIN_WR_MASK          0xFFFFFFFFu

#endif /* !PCIE_ADDR_MAP_PROVIDED */

#endif /* PCIE_ADDR_MAP_H */
