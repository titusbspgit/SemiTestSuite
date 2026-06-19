#!/usr/bin/env python3
import json
import os
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter

# Configuration
JSON_INPUT_PATH = os.environ.get("JSON_INPUT_PATH", "Test_Output/GPIO/gpio_testplan.json")
OUTPUT_DIR = "Test_Output/GPIO"
OUTPUT_XLSX = os.path.join(OUTPUT_DIR, "GPIO_TestPlan.xlsx")

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

WRAP_COL_NAMES = {
    "Test Description",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}


def load_json(path: str):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array of row objects")
    return data


def create_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Header
    header_font = Font(bold=True)
    ws.append(COLUMNS)
    for col_idx in range(1, len(COLUMNS) + 1):
        cell = ws.cell(row=1, column=col_idx)
        cell.font = header_font

    # Data rows
    for item in rows:
        row_values = []
        for col in COLUMNS:
            if col == "Code Generation (Required / Not)":
                row_values.append("")  # default empty
            else:
                row_values.append(str(item.get(col, "")))
        ws.append(row_values)

    # Freeze header row
    ws.freeze_panes = "A2"

    # Wrap text for specific columns
    wrap_alignment = Alignment(wrap_text=True, vertical="top")
    for col_idx, col_name in enumerate(COLUMNS, start=1):
        if col_name in WRAP_COL_NAMES:
            for r in range(2, ws.max_row + 1):
                ws.cell(row=r, column=col_idx).alignment = wrap_alignment

    # Borders for all cells
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(1, ws.max_row + 1):
        for c in range(1, len(COLUMNS) + 1):
            ws.cell(row=r, column=c).border = border

    # Data validation dropdown for last column across all rows
    last_col_idx = len(COLUMNS)
    last_col_letter = get_column_letter(last_col_idx)
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True)
    dv.error = "Select Required or Not Required or leave blank"
    dv.errorTitle = "Invalid Selection"
    ws.add_data_validation(dv)
    if ws.max_row < 2:
        data_range = f"{last_col_letter}2:{last_col_letter}2"
    else:
        data_range = f"{last_col_letter}2:{last_col_letter}{ws.max_row}"
    dv.add(data_range)

    # Approximate autofit for all columns
    max_widths = {i: 0 for i in range(1, len(COLUMNS) + 1)}
    for r in range(1, ws.max_row + 1):
        for c in range(1, len(COLUMNS) + 1):
            val = ws.cell(row=r, column=c).value
            s = "" if val is None else str(val)
            length = len(s)
            if length > max_widths[c]:
                max_widths[c] = length
    for c in range(1, len(COLUMNS) + 1):
        col_letter = get_column_letter(c)
        base = max_widths[c]
        # Wider defaults for wrapped columns
        col_name = COLUMNS[c - 1]
        if col_name in WRAP_COL_NAMES:
            width = min(max(30, int(base * 0.9)), 80)
        else:
            width = min(max(12, int(base * 1.1)), 60)
        ws.column_dimensions[col_letter].width = width

    # Ensure output directory exists
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Save workbook
    wb.save(OUTPUT_XLSX)

    return OUTPUT_XLSX


def main():
    rows = load_json(JSON_INPUT_PATH)
    path = create_workbook(rows)
    print(f"Generated Excel: {path}")


if __name__ == "__main__":
    main()
