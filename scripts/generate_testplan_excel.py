#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation


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
    "Code Generation (Required / Not)",
]

WRAP_COLUMNS = {
    "Test Description",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}


def timestamp_ist():
    # Format: YYYYMMDD_HHMMSS in IST (Asia/Kolkata)
    if ZoneInfo is not None:
        now = datetime.now(ZoneInfo("Asia/Kolkata"))
    else:
        # Fallback: assume local time is IST if zoneinfo missing
        now = datetime.utcnow()
    return now.strftime("%Y%m%d_%H%M%S")


def validate_json(data):
    if not isinstance(data, list):
        raise ValueError("json_data must be a list of objects")
    for i, row in enumerate(data, 1):
        if not isinstance(row, dict):
            raise ValueError(f"Element at index {i} is not an object")


def auto_fit_columns(ws, max_row, max_col):
    # Estimate column widths based on content length
    max_lengths = [0] * (max_col + 1)
    for col in range(1, max_col + 1):
        header = ws.cell(row=1, column=col).value
        max_lengths[col] = len(str(header)) if header is not None else 0
        for row in range(2, max_row + 1):
            val = ws.cell(row=row, column=col).value
            if val is None:
                continue
            length = len(str(val))
            if length > max_lengths[col]:
                max_lengths[col] = length
    for col in range(1, max_col + 1):
        col_letter = get_column_letter(col)
        header = ws.cell(row=1, column=col).value
        # Heuristic width: char count + padding, with caps
        base_width = max_lengths[col] + 2
        if header in WRAP_COLUMNS:
            width = min(max(base_width, 40), 80)
        else:
            width = min(max(base_width, 12), 50)
        ws.column_dimensions[col_letter].width = width


def apply_borders(ws, max_row, max_col):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            ws.cell(row=r, column=c).border = border


def apply_wrap_alignment(ws, max_row, header_to_col):
    wrap_alignment = Alignment(wrap_text=True, vertical="top")
    for header in WRAP_COLUMNS:
        col = header_to_col.get(header)
        if not col:
            continue
        for r in range(2, max_row + 1):
            ws.cell(row=r, column=col).alignment = wrap_alignment


def add_dropdown_validation(ws, last_col_letter, start_row=2):
    # Allow: Required, Not Required, and blanks across entire column
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True)
    # Apply to all possible rows in the sheet for that column (Excel max rows)
    dv.ranges.add(f"{last_col_letter}{start_row}:{last_col_letter}1048576")
    ws.add_data_validation(dv)


def build_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Write headers
    for col, header in enumerate(HEADERS, start=1):
        cell = ws.cell(row=1, column=col, value=header)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(vertical="center")

    # Map header to column index
    header_to_col = {h: i + 1 for i, h in enumerate(HEADERS)}

    # Populate rows; preserve values exactly; missing fields become empty cells
    for r_idx, obj in enumerate(rows, start=2):
        for h in HEADERS[:-1]:  # all except the last new column
            c_idx = header_to_col[h]
            val = obj.get(h, "")
            ws.cell(row=r_idx, column=c_idx, value=val)
        # Last column left empty by default

    max_row = ws.max_row
    max_col = len(HEADERS)

    # Freeze first row
    ws.freeze_panes = "A2"

    # Apply wrap to selected columns
    apply_wrap_alignment(ws, max_row, header_to_col)

    # Apply borders to all cells in the used range
    apply_borders(ws, max_row, max_col)

    # Data validation dropdown for last column
    last_col_letter = get_column_letter(max_col)
    add_dropdown_validation(ws, last_col_letter, start_row=2)

    # Autofit columns
    auto_fit_columns(ws, max_row, max_col)

    return wb


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    parser.add_argument('--input', required=True, help='Path to source JSON file (array of objects)')
    parser.add_argument('--output-dir', required=True, help='Directory to write the Excel file into')
    args = parser.parse_args()

    with open(args.input, 'r', encoding='utf-8') as f:
        data = json.load(f)

    validate_json(data)

    wb = build_workbook(data)

    os.makedirs(args.output_dir, exist_ok=True)

    ts = timestamp_ist()
    filename = f"testplan_{ts}.xlsx"
    out_path = os.path.join(args.output_dir, filename)
    wb.save(out_path)

    print(f"Generated: {out_path}")


if __name__ == '__main__':
    main()
