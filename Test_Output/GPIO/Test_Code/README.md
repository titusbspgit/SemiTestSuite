Generated Embedded Test Code (C, C99)

Scope
- Source: 1779867161683/testplan_20260518_221048.xlsx (sheet: TestPlan)
- Filter: Code Generation (Required / Not) == "Required"

Generated tests
- gpio_reg_wr_rd_test (folder: Test_Output/GPIO/Test_Code/gpio_reg_wr_rd_test/)

How to build
- Using Makefile (builds static library of objects):
  $ make -C Test_Output/GPIO/Test_Code clean all

Notes
- Register addresses are placeholders and must be updated per target SoC.
- printf traces are included for step-by-step diagnostics.
- Each test is isolated in its own folder with one .c and one .h file.
