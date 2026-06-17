#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generates a REAL Excel (.xlsx) TestPlan from embedded JSON and writes to Test_Output/GPIO/GPIO_TestPlan.xlsx
- Primary worksheet: GPIO_TestPlan
- Adds last column: "Code Generation (Required / Not)" with dropdown (Required/Not Required, allow blank)
- Formatting: bold header, freeze first row, wrap text for specific columns, borders, approximated autofit
- Creates a Summary sheet with IST timestamp, IP_NAME, source scope, and folder URLs
"""
from __future__ import annotations
import os
from pathlib import Path
from datetime import datetime
from zoneinfo import ZoneInfo
from typing import List, Dict, Any

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# ---------- Embedded JSON (preserve exactly) ----------
TESTPLAN_JSON: Dict[str, Any] = {
  "status": "SUCCESS",
  "data": [
    {
      "Index": "1",
      "SS / Module": "GPIO",
      "Feature": "GPIO register write/read test",
      "Test Case Name": "gpio_reg_wr_rd_test",
      "Test Description": "Validate GPIO register write and read back consistency.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "1. Initialize the GPIO module.\n2. Write known values to GPIO-related registers.\n3. Read back the written registers.\n4. Compare read values with the written values.\n5. Log pass/fail status.",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "2",
      "SS / Module": "GPIO",
      "Feature": "Negative-edge interrupt with alternate pads enabled",
      "Test Case Name": "test_gpio_nedge_alternate_pads_en",
      "Test Description": "Verify GPIO negative-edge interrupt behavior when alternate pads are enabled.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "1. Initialize the GPIO module.\n2. Enable negative-edge interrupt on alternate pads.\n3. Stimulate negative-edge transitions on enabled pads.\n4. Monitor and record interrupt events.\n5. Report results.",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "3",
      "SS / Module": "GPIO",
      "Feature": "Negative-edge interrupt enable",
      "Test Case Name": "test_gpio_negedge_intr_en",
      "Test Description": "Check that enabling negative-edge interrupts triggers events on falling edges.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "1. Initialize the GPIO module.\n2. Configure negative-edge interrupt on selected pads.\n3. Apply falling-edge signals to the pads.\n4. Observe interrupt signaling and capture status.\n5. Summarize pass/fail.",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "4",
      "SS / Module": "GPIO",
      "Feature": "Positive-edge interrupt with all pads enabled",
      "Test Case Name": "test_gpio_pedge_all_pads_en",
      "Test Description": "Validate GPIO positive-edge interrupts when enabled on all pads.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "1. Initialize the GPIO module.\n2. Enable positive-edge interrupt on all pads.\n3. Generate rising edges on multiple pads.\n4. Observe interrupt activity and log events.\n5. Determine overall result.",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "5",
      "SS / Module": "GPIO",
      "Feature": "Positive-edge interrupt with alternate pads enabled",
      "Test Case Name": "test_gpio_pedge_alternate_pads_en",
      "Test Description": "Verify positive-edge interrupt functionality when alternate pads are enabled.",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "1. Initialize the GPIO module.\n2. Enable positive-edge interrupt on alternate pads.\n3. Apply rising-edge signals to enabled pads.\n4. Monitor interrupt occurrences and capture status.\n5. Conclude pass/fail.",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    }
  ],
  "errors": []
}

IP_NAME = "GPIO"
SOURCE_SCOPE = "TestRepo/gpio"
SOURCE_FOLDERS: List[str] = [
    "https://github.com/titusbspgit/SemiTestSuite/tree/main/TestRepo/gpio/gpio_reg_wr_rd_test",
    "https://github.com/titusbspgit/SemiTestSuite/tree/main/TestRepo/gpio/test_gpio_nedge_alternate_pads_en",
    "https://github.com/titusbspgit/SemiTestSuite/tree/main/TestRepo/gpio/test_gpio_negedge_intr_en",
    "https://github.com/titusbspgit/SemiTestSuite/tree/main/TestRepo/gpio/test_gpio_pedge_all_pads_en",
    "https://github.com/titusbspgit/SemiTestSuite/tree/main/TestRepo/gpio/test_gpio_pedge_alternate_pads_en",
]

OUTPUT_DIR = Path("Test_Output/GPIO")
OUTPUT_FILE = OUTPUT_DIR / "GPIO_TestPlan.xlsx"
PRIMARY_SHEET_NAME = "GPIO_TestPlan"
SUMMARY_SHEET_NAME = "Summary"

HEADERS: List[str] = [
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

WRAP_COL_INDICES = {5, 9, 11}  # 1-based indices: E, I, K


def validate_json() -> List[Dict[str, Any]]:
    if not isinstance(TESTPLAN_JSON, dict):
        raise ValueError("Top-level JSON must be an object with keys 'status' and 'data'.")
    if TESTPLAN_JSON.get("status") != "SUCCESS":
        raise ValueError("JSON status must be 'SUCCESS'.")
    data = TESTPLAN_JSON.get("data")
    if not isinstance(data, list):
        raise ValueError("'data' must be an array of row objects.")
    for i, row in enumerate(data, start=1):
        if not isinstance(row, dict):
            raise ValueError(f"Row {i} is not an object.")
    return data


def build_workbook(rows: List[Dict[str, Any]]) -> Workbook:
    wb = Workbook()
    # Remove default sheet
    default_ws = wb.active
    wb.remove(default_ws)

    ws = wb.create_sheet(PRIMARY_SHEET_NAME)

    # Header row with bold font
    header_font = Font(bold=True)
    for col_idx, header in enumerate(HEADERS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=header)
        cell.font = header_font

    # Write data rows
    for r_idx, row_obj in enumerate(rows, start=2):
        for c_idx, header in enumerate(HEADERS, start=1):
            if header == "Code Generation (Required / Not)":
                value = ""  # default empty
            else:
                value = row_obj.get(header, "")
            cell = ws.cell(row=r_idx, column=c_idx, value=value)
            # Wrap text for specified columns
            if c_idx in WRAP_COL_INDICES:
                cell.alignment = Alignment(wrap_text=True, vertical="top")

    # Freeze first row
    ws.freeze_panes = "A2"

    # Apply data validation dropdown to last column for all rows
    last_col_letter = ws.cell(row=1, column=len(HEADERS)).column_letter
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True)
    ws.add_data_validation(dv)
    dv.add(f"{last_col_letter}2:{last_col_letter}1048576")  # entire column (except header)

    # Apply borders to all used cells
    thin = Side(style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    max_row = ws.max_row
    max_col = ws.max_column
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            ws.cell(row=r, column=c).border = border

    # Approximate autofit for all columns
    col_widths = {i: len(HEADERS[i-1]) + 2 for i in range(1, len(HEADERS) + 1)}
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            v = ws.cell(row=r, column=c).value
            if v is None:
                continue
            text = str(v)
            if c in WRAP_COL_INDICES:
                # Consider the longest line for width
                units = max(len(line) for line in text.splitlines()) if text else 0
            else:
                units = len(text)
            if units > col_widths[c]:
                col_widths[c] = units
    for c in range(1, ws.max_column + 1):
        letter = ws.cell(row=1, column=c).column_letter
        # Heuristic scaling for pixel-to-character width; cap to keep reasonable
        width = min(max(col_widths[c] + 2, 12), 80)
        ws.column_dimensions[letter].width = width

    # Add Summary sheet
    summary = wb.create_sheet(SUMMARY_SHEET_NAME)
    ist_now = datetime.now(ZoneInfo("Asia/Kolkata"))
    summary["A1"] = "TestPlan Generation Summary"
    summary["A1"].font = Font(bold=True)

    meta = [
        ("Generated At (IST)", ist_now.strftime("%Y-%m-%d %H:%M:%S %Z")),
        ("IP_NAME", IP_NAME),
        ("Source Scope", SOURCE_SCOPE),
        ("Output File", str(OUTPUT_FILE)),
    ]
    r = 3
    for k, v in meta:
        summary.cell(row=r, column=1, value=k).font = Font(bold=True)
        summary.cell(row=r, column=2, value=v)
        r += 1

    summary.cell(row=r, column=1, value="Source Folders").font = Font(bold=True)
    r += 1
    for url in SOURCE_FOLDERS:
        summary.cell(row=r, column=1, value=url)
        r += 1

    return wb


def main() -> None:
    rows = validate_json()
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    wb = build_workbook(rows)
    wb.save(str(OUTPUT_FILE))
    print(f"Wrote Excel to: {OUTPUT_FILE}")


if __name__ == "__main__":
    main()
