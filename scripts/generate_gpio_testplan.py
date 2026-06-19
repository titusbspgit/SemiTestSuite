#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
GPIO TestPlan Excel Generator
- Consumes embedded JSON (authoritative)
- Generates REAL .xlsx at Test_Output/GPIO/GPIO_TestPlan.xlsx
- Formatting: bold header, freeze first row, borders, wrap text, approximate autofit
- Adds data validation dropdown for last column: [Required, Not Required], allow blank
"""
import json
import os
import sys
from typing import List, Dict

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# -------------------- Authoritative JSON (DO NOT MODIFY) --------------------
JSON_DATA = r'''{
  "status": "SUCCESS",
  "data": [
    {
      "Index": "1",
      "SS / Module": "GPIO",
      "Feature": "Register write/read on GPIOA ODR (PA0)",
      "Test Case Name": "gpio_reg_wr_rd_test",
      "Test Description": "Validates GPIOA clock enable, PA0 output configuration, and write/read of ODR bit0 by setting high then low and checking readback.",
      "Speed": "NA",
      "Mode": "Polling",
      "Remarks": "Requires target with RCC and GPIOA at specified memory map; uses ITM for PASS/FAIL prints; program halts in an infinite loop after PASS.",
      "Test Steps / Procedure": "1. Enable AHB1 clock for GPIOA via RCC_AHB1ENR.\n2. Configure PA0 as output by updating GPIOA_MODER.\n3. Set PA0 high by writing GPIOA_ODR bit0.\n4. Read back GPIOA_ODR bit0 and verify it is set.\n5. Clear PA0 low by clearing GPIOA_ODR bit0.\n6. Read back GPIOA_ODR bit0 and verify it is cleared.",
      "Impacted Registers": "RCC_AHB1ENR, GPIOA_MODER, GPIOA_ODR",
      "Validation / Acceptance Criteria": "PASS if (GPIOA_ODR & (1<<0)) is true after setting high and (GPIOA_ODR & (1<<0)) is false after clearing low; otherwise prints FAIL and halts.",
      "Gap Analysis": "- No verification of actual pad state beyond ODR readback.\n- Hardcoded pin (PA0) and addresses; no parameterization.\n- No negative/error handling or cleanup."
    },
    {
      "Index": "2",
      "SS / Module": "GPIO",
      "Feature": "Falling-edge trigger enable on alternate EXTI lines",
      "Test Case Name": "test_gpio_nedge_alternate_pads_en",
      "Test Description": "Enables EXTI falling-edge trigger for alternate lines (0,2,4,6) and verifies configuration via readback.",
      "Speed": "NA",
      "Mode": "Polling",
      "Remarks": "Assumes EXTI block accessible and lines 0,2,4,6 are valid; uses ITM for PASS/FAIL prints; program halts after PASS.",
      "Test Steps / Procedure": "1. For pins 0, 2, 4, 6, set the corresponding bit in EXTI_FTSR.\n2. After each write, read EXTI_FTSR and verify the bit is set.\n3. If any verification fails, declare FAIL; otherwise declare PASS.\n4. Enter infinite loop after PASS.",
      "Impacted Registers": "EXTI_FTSR",
      "Validation / Acceptance Criteria": "PASS if for each of pins {0,2,4,6}, (EXTI_FTSR & (1<<pin)) evaluates true after enabling; otherwise prints FAIL and halts.",
      "Gap Analysis": "- No IMR configuration or source pin mapping validated.\n- Hardcoded pin list; no boundary or error cases.\n- Does not test actual interrupt/event occurrence."
    },
    {
      "Index": "3",
      "SS / Module": "GPIO",
      "Feature": "Negative-edge interrupt configuration on EXTI1 (PA1)",
      "Test Case Name": "test_gpio_negedge_intr_en",
      "Test Description": "Enables clocks, configures PA1 as input and maps to EXTI1, enables IMR and falling-edge trigger, then verifies EXTI1 FTSR bit.",
      "Speed": "NA",
      "Mode": "Polling",
      "Remarks": "Requires RCC AHB1/APB2 and SYSCFG access; assumes PA1 is mapped to EXTI1; uses ITM for PASS/FAIL prints; program halts after PASS.",
      "Test Steps / Procedure": "1. Enable GPIOA and SYSCFG clocks via RCC_AHB1ENR and RCC_APB2ENR.\n2. Configure PA1 as input via GPIOA_MODER.\n3. Map PA1 to EXTI1 by clearing SYSCFG_EXTICR1 field for EXTI1.\n4. Enable EXTI1 in EXTI_IMR and set falling-edge in EXTI_FTSR.\n5. Verify EXTI_FTSR bit1 is set for line 1.\n6. Declare PASS if set; otherwise FAIL and halt.",
      "Impacted Registers": "RCC_AHB1ENR, RCC_APB2ENR, GPIOA_MODER, SYSCFG_EXTICR1, EXTI_IMR, EXTI_FTSR",
      "Validation / Acceptance Criteria": "PASS if (EXTI_FTSR & (1<<1)) evaluates true after configuration; otherwise prints \"FAIL: NEG EDGE NOT ENABLED\" and halts.",
      "Gap Analysis": "- No NVIC/ISR configuration or interrupt servicing to confirm runtime behavior.\n- Limited to FTSR readback; IMR/mapping not independently verified beyond writes.\n- Hardcoded line (EXTI1/PA1) and addresses."
    },
    {
      "Index": "4",
      "SS / Module": "GPIO",
      "Feature": "Rising-edge trigger enable on all EXTI lines 0–7",
      "Test Case Name": "test_gpio_pedge_all_pads_en",
      "Test Description": "Enables EXTI rising-edge trigger for lines 0 through 7 and verifies each via readback.",
      "Speed": "NA",
      "Mode": "Polling",
      "Remarks": "Assumes EXTI lines 0–7 are present and writable; uses ITM for PASS/FAIL prints; program halts after PASS.",
      "Test Steps / Procedure": "1. For i from 0 to 7, set the corresponding bit in EXTI_RTSR.\n2. After each write, read EXTI_RTSR and verify the bit is set.\n3. If any verification fails, declare FAIL; otherwise declare PASS.\n4. Enter infinite loop after PASS.",
      "Impacted Registers": "EXTI_RTSR",
      "Validation / Acceptance Criteria": "PASS if for each i in [0..7], (EXTI_RTSR & (1<<i)) evaluates true after enabling; otherwise prints FAIL and halts.",
      "Gap Analysis": "- No IMR configuration or pin-to-line mapping validation.\n- Does not verify actual event generation; configuration-only.\n- Hardcoded line range (0–7); no bounds or error handling."
    },
    {
      "Index": "5",
      "SS / Module": "GPIO",
      "Feature": "Rising-edge trigger enable on alternate EXTI lines",
      "Test Case Name": "test_gpio_pedge_alternate_pads_en",
      "Test Description": "Enables EXTI rising-edge trigger for alternate lines (0,2,4,6) and verifies configuration via readback.",
      "Speed": "NA",
      "Mode": "Polling",
      "Remarks": "Assumes EXTI block accessible and lines 0,2,4,6 valid; uses ITM for PASS/FAIL prints; program halts after PASS.",
      "Test Steps / Procedure": "1. For pins 0, 2, 4, 6, set the corresponding bit in EXTI_RTSR.\n2. After each write, read EXTI_RTSR and verify the bit is set.\n3. If any verification fails, declare FAIL; otherwise declare PASS.\n4. Enter infinite loop after PASS.",
      "Impacted Registers": "EXTI_RTSR",
      "Validation / Acceptance Criteria": "PASS if for each of pins {0,2,4,6}, (EXTI_RTSR & (1<<pin)) evaluates true after enabling; otherwise prints FAIL and halts.",
      "Gap Analysis": "- Configuration-only; no end-to-end interrupt/event validation.\n- Hardcoded pin set; no negative or boundary testing.\n- No cleanup or restoration of default states."
    }
  ],
  "errors": []
}'''
# ---------------------------------------------------------------------------

OUTPUT_PATH = os.path.join("Test_Output", "GPIO", "GPIO_TestPlan.xlsx")
SHEET_NAME = "TestPlan"
COLUMNS: List[str] = [
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
    "Code Generation (Required / Not)",
]
WRAP_COLS = {"Test Description", "Test Steps / Procedure", "Validation / Acceptance Criteria"}

def validate_json(payload: Dict) -> List[Dict]:
    if not isinstance(payload, dict):
        raise ValueError("Top-level JSON must be an object")
    if payload.get("status") != "SUCCESS":
        raise ValueError("JSON status must be 'SUCCESS'")
    data = payload.get("data")
    if not isinstance(data, list):
        raise ValueError("'data' must be an array of row objects")
    for i, row in enumerate(data, 1):
        if not isinstance(row, dict):
            raise ValueError(f"Row {i} is not an object")
    return data


def autofit_columns(ws):
    # Approximate autofit based on max string length
    for col_idx, col_name in enumerate(COLUMNS, start=1):
        max_len = len(col_name)
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            cell = row[0]
            val = cell.value
            if val is None:
                continue
            s = str(val)
            if len(s) > max_len:
                max_len = len(s)
        # Heuristic width; wrap columns can be narrower
        base = 1.2
        width = min(120, max(12 if col_name in WRAP_COLS else 18, int(max_len * base)))
        ws.column_dimensions[cell.column_letter].width = width


def main():
    try:
        payload = json.loads(JSON_DATA)
        rows = validate_json(payload)
    except Exception as e:
        print(f"ERROR: Invalid JSON input: {e}", file=sys.stderr)
        sys.exit(2)

    wb = Workbook()
    ws = wb.active
    ws.title = SHEET_NAME

    # Header
    header_font = Font(bold=True)
    thin = Side(style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)

    ws.append(COLUMNS)
    for col_idx in range(1, len(COLUMNS) + 1):
        c = ws.cell(row=1, column=col_idx)
        c.font = header_font
        c.border = border

    # Rows
    for r_idx, row in enumerate(rows, start=2):
        values = [row.get(col, "") for col in COLUMNS[:-1]] + [""]
        ws.append(values)
        for c_idx, col_name in enumerate(COLUMNS, start=1):
            cell = ws.cell(row=r_idx, column=c_idx)
            cell.border = border
            if col_name in WRAP_COLS:
                cell.alignment = Alignment(wrap_text=True, vertical="top")
            else:
                cell.alignment = Alignment(vertical="top")

    # Freeze top row
    ws.freeze_panes = "A2"

    # Data validation for last column
    last_col_letter = ws.cell(row=1, column=len(COLUMNS)).column_letter
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=True)
    dv.error = "Select from the list"
    dv.prompt = "Choose Required or Not Required (blank allowed)"
    ws.add_data_validation(dv)
    dv.add(f"{last_col_letter}2:{last_col_letter}{ws.max_row}")

    # Autofit columns (approx)
    autofit_columns(ws)

    # Ensure output dir exists
    out_dir = os.path.dirname(OUTPUT_PATH)
    os.makedirs(out_dir, exist_ok=True)

    # Save workbook
    wb.save(OUTPUT_PATH)
    print(f"Saved Excel to {OUTPUT_PATH}")


if __name__ == "__main__":
    main()

# retrigger
