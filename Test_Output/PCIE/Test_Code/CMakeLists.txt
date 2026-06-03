cmake_minimum_required(VERSION 3.10)
project(pcie_tests C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/unity
    ${CMAKE_CURRENT_SOURCE_DIR}/mock
    ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_library(unity STATIC vendor/unity/unity.c)
add_library(mock_mmio STATIC mock/mock_mmio.c)

set(TEST_SOURCES
    tests/tests_main.c
    tests/pcie_cfg_wr_rd_test/pcie_cfg_wr_rd_test.c
    tests/pcie_dbi_dsp_reg_wr_rd_test/pcie_dbi_dsp_reg_wr_rd_test.c
    tests/pcie_dbi_usp_reg_wr_rd_test/pcie_dbi_usp_reg_wr_rd_test.c
    tests/pcie_sii_rc_reg_wr_rd_test/pcie_sii_rc_reg_wr_rd_test.c
)

add_executable(pcie_tests ${TEST_SOURCES})

target_link_libraries(pcie_tests unity mock_mmio)

# Optionally, allow providing a real map: -DPCIE_ADDR_MAP_PROVIDED
# Users can -DPCIE_ADDR_MAP_PROVIDED and supply an alternate include path/header.
