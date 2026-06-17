#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

COLUMNS = [
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

WRAP_COLS = {5, 9, 11}  # 1-based column indexes for wrap text


def validate_json(data):
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array of row objects")
    for i, row in enumerate(data, start=1):
        if not isinstance(row, dict):
            raise ValueError(f"Element at index {i} is not an object")


def autosize_columns(ws, max_row, max_col):
    # Compute max length per column
    max_len = {c: 0 for c in range(1, max_col + 1)}
    for col in range(1, max_col + 1):
        header = str(ws.cell(row=1, column=col).value or "")
        max_len[col] = max(max_len[col], len(header))
        for r in range(2, max_row + 1):
            val = ws.cell(row=r, column=col).value
            s = str(val) if val is not None else ""
            # Limit size to avoid huge widths
            if len(s) > 200:
                s = s[:200]
            max_len[col] = max(max_len[col], len(s))

    for col in range(1, max_col + 1):
        letter = get_column_letter(col)
        base = max_len[col]
        # Approximate width: character count * 1.2 with bounds
        width = max(12, min(int(base * 1.2) + 2, 100))
        # For wrapped columns, give a wider default
        if col in WRAP_COLS:
            width = max(width, 60)
        ws.column_dimensions[letter].width = width


def apply_borders(ws, max_row, max_col):
    thin = Side(style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            ws.cell(row=r, column=c).border = border


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    parser.add_argument("--input", required=True, help="Path to input JSON array")
    parser.add_argument("--output-dir", required=True, help="Directory to write the Excel file")
    args = parser.parse_args()

    with open(args.input, "r", encoding="utf-8") as f:
        data = json.load(f)

    validate_json(data)

    os.makedirs(args.output_dir, exist_ok=True)

    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Header row
    for col_idx, header in enumerate(COLUMNS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=header)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(vertical="top", wrap_text=True)

    # Data rows
    for r_idx, row in enumerate(data, start=2):
        for c_idx, header in enumerate(COLUMNS, start=1):
            if header == "Code Generation (Required / Not)":
                value = ""  # default empty
            else:
                # Preserve exactly as string where possible
                value = row.get(header, "")
            cell = ws.cell(row=r_idx, column=c_idx, value=value)
            if c_idx in WRAP_COLS:
                cell.alignment = Alignment(wrap_text=True, vertical="top")
            else:
                cell.alignment = Alignment(vertical="top")

    # Freeze first row
    ws.freeze_panes = "A2"

    max_row = ws.max_row
    max_col = ws.max_column

    # Data validation for last column
    last_col_letter = get_column_letter(max_col)
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True)
    dv.error = "Select a value from the list"
    dv.prompt = "Choose \"Required\" or \"Not Required\" (or leave blank)"
    ws.add_data_validation(dv)
    dv.add(f"{last_col_letter}2:{last_col_letter}{max_row}")

    # Borders
    apply_borders(ws, max_row, max_col)

    # Autosize columns
    autosize_columns(ws, max_row, max_col)

    # IST timestamp
    ist_now = datetime.now(ZoneInfo("Asia/Kolkata"))
    ts = ist_now.strftime("%Y%m%d_%H%M%S")
    out_path = os.path.join(args.output_dir, f"testplan_{ts}.xlsx")

    wb.save(out_path)
    print(out_path)

if __name__ == "__main__":
    main()
