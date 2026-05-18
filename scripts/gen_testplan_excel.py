import os
import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

# STEP 1 — Validate JSON (embedded exact JSON data)
json_data = [
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "GPIO output data register write/read on PA0",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Enables GPIOA clock, configures PA0 as output, toggles the output data register, and verifies the write by reading back GPIOA_ODR.",
    "Speed": "NA",
    "Mode": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1) Enable GPIOA bus clock. 2) Configure PA0 as a general-purpose output. 3) Set PA0 output high via GPIOA_ODR. 4) Read back GPIOA_ODR to confirm PA0 is high. 5) Set PA0 output low via GPIOA_ODR. 6) Read back GPIOA_ODR to confirm PA0 is low. 7) Declare test pass on successful checks.",
    "Impacted Registers": "RCC_AHB1ENR, GPIOA_MODER, GPIOA_ODR",
    "Validation / Acceptance Criteria": "After setting, GPIOA_ODR bit 0 must read 1; after clearing, GPIOA_ODR bit 0 must read 0. Any mismatch triggers FAIL.",
    "Gap Analysis": "Does not verify actual pad voltage or input readback; no timing or drive-strength checks; only tests PA0."
  },
  {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "EXTI falling-edge trigger enable on alternate pins 0,2,4,6",
    "Test Case Name": "test_gpio_nedge_alternate_pads_en",
    "Test Description": "Iterates over pins 0, 2, 4, and 6 to enable EXTI falling-edge triggers and verifies each enable by reading back EXTI_FTSR.",
    "Speed": "NA",
    "Mode": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1) Select pins 0, 2, 4, and 6. 2) Enable falling-edge trigger for each selected EXTI line. 3) Read back EXTI_FTSR to confirm each bit is set. 4) Declare test pass when all checks succeed.",
    "Impacted Registers": "EXTI_FTSR",
    "Validation / Acceptance Criteria": "EXTI_FTSR bits for lines 0, 2, 4, and 6 must read 1 after configuration; any unset bit triggers FAIL.",
    "Gap Analysis": "Only verifies configuration bits; does not generate falling edges, check pending flags, unmask interrupts, or verify ISR behavior."
  },
  {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "EXTI1 falling-edge interrupt path configuration on PA1",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Enables required clocks, sets PA1 as input, maps EXTI1 to PA1, unmasks EXTI1, enables falling-edge trigger, and verifies by reading back EXTI_FTSR.",
    "Speed": "NA",
    "Mode": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1) Enable AHB1 and APB2 clocks for GPIOA and system configuration. 2) Configure PA1 as input. 3) Map EXTI1 to PA1 via system configuration register. 4) Unmask EXTI1 in EXTI_IMR. 5) Enable falling-edge trigger for EXTI1. 6) Read back EXTI_FTSR to confirm falling-edge is enabled. 7) Declare test pass when the check succeeds.",
    "Impacted Registers": "RCC_AHB1ENR, RCC_APB2ENR, GPIOA_MODER, SYSCFG_EXTICR1, EXTI_IMR, EXTI_FTSR",
    "Validation / Acceptance Criteria": "EXTI_FTSR bit for line 1 must read 1 after configuration; mismatch triggers FAIL.",
    "Gap Analysis": "No NVIC or ISR setup; does not generate a falling edge or verify pending flags/interrupt handling beyond configuration readback."
  },
  {
    "Index": "4",
    "SS / Module": "GPIO",
    "Feature": "EXTI rising-edge trigger enable on lines 0–7",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Enables rising-edge triggers for EXTI lines 0 through 7 and verifies each enable by reading back EXTI_RTSR.",
    "Speed": "NA",
    "Mode": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1) Iterate EXTI lines 0 to 7. 2) Enable rising-edge trigger for each line. 3) Read back EXTI_RTSR to confirm each bit is set. 4) Declare test pass when all checks succeed.",
    "Impacted Registers": "EXTI_RTSR",
    "Validation / Acceptance Criteria": "For each EXTI line 0–7, the corresponding EXTI_RTSR bit must read 1 after configuration; any unset bit triggers FAIL.",
    "Gap Analysis": "Only verifies configuration bits; does not generate rising edges, check pending flags, unmask interrupts, or verify ISR behavior."
  },
  {
    "Index": "5",
    "SS / Module": "GPIO",
    "Feature": "EXTI rising-edge trigger enable on alternate pins 0,2,4,6",
    "Test Case Name": "test_gpio_pedge_alternate_pads_en",
    "Test Description": "Iterates over pins 0, 2, 4, and 6 to enable EXTI rising-edge triggers and verifies each enable by reading back EXTI_RTSR.",
    "Speed": "NA",
    "Mode": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1) Select pins 0, 2, 4, and 6. 2) Enable rising-edge trigger for each selected EXTI line. 3) Read back EXTI_RTSR to confirm each bit is set. 4) Declare test pass when all checks succeed.",
    "Impacted Registers": "EXTI_RTSR",
    "Validation / Acceptance Criteria": "EXTI_RTSR bits for lines 0, 2, 4, and 6 must read 1 after configuration; any unset bit triggers FAIL.",
    "Gap Analysis": "Only verifies configuration bits; does not generate rising edges, check pending flags, unmask interrupts, or verify ISR behavior."
  }
]

if not isinstance(json_data, list):
    raise SystemExit("json_data must be a list/array of rows")

# STEP 2 — Attempt Direct Excel Generation (using openpyxl)
columns = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Test Description",
    "Speed",
    "Mode",
    "Remarks",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Gap Analysis",
]

wb = Workbook()
ws = wb.active
ws.title = "TestPlan"

# Header
bold = Font(bold=True)
ws.append(columns)
for cell in ws[1]:
    cell.font = bold

# Freeze first row
ws.freeze_panes = "A2"

# Rows
for item in json_data:
    if not isinstance(item, dict):
        raise SystemExit("Each item in json_data must be an object/dict")
    row = [item.get(col, "") for col in columns]
    ws.append(row)

# STEP 3 — Save File with IST timestamp
ist_now = datetime.datetime.now(ZoneInfo("Asia/Kolkata"))
stamp = ist_now.strftime("%Y%m%d_%H%M%S")
output_dir = os.path.join("Test_Output", "GPIO")
os.makedirs(output_dir, exist_ok=True)
filename = f"testplan_{stamp}.xlsx"
output_path = os.path.join(output_dir, filename)
wb.save(output_path)

print(f"Generated Excel: {output_path}")
