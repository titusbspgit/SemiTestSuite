#!/usr/bin/env python3
# noop-change: trigger workflow run
import json
import os
from datetime import datetime
import pytz
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment

# Constants
INPUT_JSON_PATH = os.path.join('Test_Output', 'GPIO', 'testplan_data.json')
OUTPUT_DIR = os.path.join('Test_Output', 'GPIO')
SHEET_NAME = 'TestPlan'
HEADERS = [
    'Index',
    'SS / Module',
    'Feature',
    'Test Case Name',
    'Test Description',
    'Speed',
    'Mode',
    'Remarks',
    'Test Steps / Procedure',
    'Impacted Registers',
    'Validation / Acceptance Criteria',
    'Gap Analysis'
]


def main():
    # Step 1 — Validate JSON
    with open(INPUT_JSON_PATH, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError('Input JSON must be an array of row objects.')

    # Ensure no data loss: verify all required keys exist in every row
    for i, row in enumerate(data, 1):
        if not isinstance(row, dict):
            raise ValueError(f'Row {i} is not an object.')
        for h in HEADERS:
            if h not in row:
                # Fill missing columns with 'NA' to keep structure stable
                row[h] = 'NA'

    # Step 2 — Create workbook and sheet
    wb = Workbook()
    ws = wb.active
    ws.title = SHEET_NAME

    # Header row
    ws.append(HEADERS)
    header_font = Font(bold=True)
    for col_idx in range(1, len(HEADERS) + 1):
        cell = ws.cell(row=1, column=col_idx)
        cell.font = header_font
        cell.alignment = Alignment(wrap_text=True, vertical='top')

    # Data rows
    for row in data:
        values = [row.get(h, 'NA') for h in HEADERS]
        ws.append(values)

    # Formatting: wrap text and align top for all data cells
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=len(HEADERS)):
        for cell in row:
            cell.alignment = Alignment(wrap_text=True, vertical='top')

    # Freeze first row
    ws.freeze_panes = 'A2'

    # Set some reasonable column widths
    widths = {
        'A': 8,   # Index
        'B': 14,  # SS / Module
        'C': 28,  # Feature
        'D': 36,  # Test Case Name
        'E': 46,  # Test Description
        'F': 10,  # Speed
        'G': 10,  # Mode
        'H': 22,  # Remarks
        'I': 60,  # Test Steps / Procedure
        'J': 28,  # Impacted Registers
        'K': 56,  # Validation / Acceptance Criteria
        'L': 56,  # Gap Analysis
    }
    for col, width in widths.items():
        ws.column_dimensions[col].width = width

    # Step 3 — Save file with IST timestamp
    ist = pytz.timezone('Asia/Kolkata')
    ts = datetime.now(ist).strftime('%Y%m%d_%H%M%S')
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    out_path = os.path.join(OUTPUT_DIR, f'testplan_{ts}.xlsx')
    wb.save(out_path)
    print(f'Wrote Excel: {out_path}')


if __name__ == '__main__':
    main()
