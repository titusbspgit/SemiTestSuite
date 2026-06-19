#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate a REAL .xlsx Test Plan from embedded JSON.
- Sheet name: TestPlan
- Columns order fixed, with a final dropdown column: "Code Generation (Required / Not)"
- Formatting: bold header, freeze first row, wrap text in long columns, borders, auto-fit widths
- Output path: Test_Output/GPIO/GPIO_TestPlan.xlsx

This script is deterministic and reads no external inputs.
"""

import json
import os
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# Embedded JSON (use as-is, do not transform values)
JSON_DATA = r'''{
  "status": "SUCCESS",
  "data": [
    {
      "Index": "1",
      "SS / Module": "GPIO",
      "Feature": "gpio_reg_wr_rd",
      "Test Case Name": "gpio_reg_wr_rd_test",
      "Test Description": "Validate GPIO register write and read behavior.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "NA",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "2",
      "SS / Module": "GPIO",
      "Feature": "nedge_alternate_pads_en",
      "Test Case Name": "test_gpio_nedge_alternate_pads_en",
      "Test Description": "Verify GPIO negative-edge behavior on alternate pads when enabled.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "NA",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "3",
      "SS / Module": "GPIO",
      "Feature": "negedge_intr_en",
      "Test Case Name": "test_gpio_negedge_intr_en",
      "Test Description": "Verify GPIO negative-edge interrupt enable functionality.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "NA",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "4",
      "SS / Module": "GPIO",
      "Feature": "pedge_all_pads_en",
      "Test Case Name": "test_gpio_pedge_all_pads_en",
      "Test Description": "Verify GPIO positive-edge behavior with all pads enabled.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "NA",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "5",
      "SS / Module": "GPIO",
      "Feature": "pedge_alternate_pads_en",
      "Test Case Name": "test_gpio_pedge_alternate_pads_en",
      "Test Description": "Verify GPIO positive-edge behavior on alternate pads when enabled.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "NA",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    }
  ],
  "errors": []
}'''

HEADERS = [
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
    "Code Generation (Required / Not)",  # new last column
]

WRAP_COLS = {
    "Test Description",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

OUTPUT_PATH = Path("Test_Output/GPIO/GPIO_TestPlan.xlsx")


def validate_json(payload: dict) -> list:
    if not isinstance(payload, dict):
        raise ValueError("json_data must be a JSON object with key 'data'")
    if 'data' not in payload:
        raise ValueError("json_data missing 'data' key")
    rows = payload['data']
    if not isinstance(rows, list):
        raise ValueError("json_data['data'] must be an array of row objects")
    for i, row in enumerate(rows, start=1):
        if not isinstance(row, dict):
            raise ValueError(f"Row {i} is not an object")
    return rows


def auto_fit_column_widths(ws):
    # Estimate width based on max string length in each column (cap to a reasonable width)
    max_widths = {}
    for col_idx, header in enumerate(HEADERS, start=1):
        max_width = len(str(header)) + 2
        for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=col_idx, max_col=col_idx):
            cell = row[0]
            if cell.value is None:
                continue
            # Convert to string and consider line breaks
            text = str(cell.value)
            for line in text.splitlines() or [text]:
                if len(line) > max_width:
                    max_width = len(line)
        # Reasonable bounds
        max_width = max(10, min(max_width, 80))
        max_widths[col_idx] = max_width
    for col_idx, width in max_widths.items():
        col_letter = ws.cell(row=1, column=col_idx).column_letter
        ws.column_dimensions[col_letter].width = width


def apply_formatting(ws):
    # Header bold and freeze first row
    header_font = Font(bold=True)
    for col_idx in range(1, len(HEADERS) + 1):
        ws.cell(row=1, column=col_idx).font = header_font
    ws.freeze_panes = "A2"

    # Wrap text in specified columns
    header_to_colidx = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    wrap_alignment = Alignment(wrap_text=True, vertical="top")
    for header in WRAP_COLS:
        cidx = header_to_colidx.get(header)
        if cidx:
            for r in range(2, ws.max_row + 1):
                ws.cell(row=r, column=cidx).alignment = wrap_alignment

    # Borders for all populated cells
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border

    # Auto-fit columns
    auto_fit_column_widths(ws)


def add_dropdown_validation(ws):
    last_col_idx = ws.max_column
    last_col_letter = ws.cell(row=1, column=last_col_idx).column_letter
    # Dropdown list: Required, Not Required; allow blanks
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True)
    ws.add_data_validation(dv)
    # Apply to all data rows in the last column (entire column except header)
    dv.add(f"{last_col_letter}2:{last_col_letter}1048576")


def build_workbook(rows: list) -> Workbook:
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Write header
    for col_idx, header in enumerate(HEADERS, start=1):
        ws.cell(row=1, column=col_idx, value=header)

    # Write data rows preserving values exactly
    for r_idx, item in enumerate(rows, start=2):
        for c_idx, header in enumerate(HEADERS, start=1):
            if header == "Code Generation (Required / Not)":
                value = ""  # default empty
            else:
                value = item.get(header, "")
            ws.cell(row=r_idx, column=c_idx, value=value)

    # Apply validation and formatting
    add_dropdown_validation(ws)
    apply_formatting(ws)

    return wb


def main():
    payload = json.loads(JSON_DATA)
    rows = validate_json(payload)

    # Ensure output directory exists
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)

    wb = build_workbook(rows)
    wb.save(OUTPUT_PATH)
    print(f"WROTE: {OUTPUT_PATH}")


if __name__ == "__main__":
    main()
