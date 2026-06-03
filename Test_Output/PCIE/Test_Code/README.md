PCIE Test Code (Generated)

Overview
- Generated from 1780469325123/PCIE_TestPlan.xlsx.
- Device-agnostic by default; uses a mock MMIO backend.
- Provide a real device address map by defining PCIE_ADDR_MAP_PROVIDED and supplying concrete addresses/masks in include/pcie_addr_map.h or via your build system.

Layout
- include/pcie_addr_map.h: Symbolic register base/offsets and default masks (placeholders) with a PCIE_ADDR_MAP_PROVIDED guard.
- mock/: In-memory MMIO emulation supporting 8/16/32-bit accesses.
- vendor/unity/: Minimal Unity-like harness used by tests.
- tests/: One folder per test case with <name>.c and <name>.h, plus a central tests_main.c runner with optional substring filter via CLI.

Running
- POSIX: ./run_tests.sh [optional-filter]
- Manual:
  mkdir -p build && cd build && cmake .. && cmake --build . && ./pcie_tests [optional-filter]

Providing a real device map
- Edit include/pcie_addr_map.h with real base/offsets and field masks and build with -DPCIE_ADDR_MAP_PROVIDED.
- Alternatively, keep placeholders for mock-only execution.

Notes
- Tests that require device-specific RO/RW masks (e.g., pcie_cfg_wr_rd_test) are marked ignored unless PCIE_ADDR_MAP_PROVIDED is set.
- Other tests validate default 0 values and read/write behavior using the mock.
