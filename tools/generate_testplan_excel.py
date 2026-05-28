#!/usr/bin/env python3
import json
import os
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter

REPO_ROOT = Path(__file__).resolve().parents[1]
INPUT_JSON = REPO_ROOT / 'Test_Output' / 'PCIE' / 'pcie_testplan_input.json'
OUTPUT_XLSX = REPO_ROOT / 'Test_Output' / 'PCIE' / 'PCIE_TestPlan.xlsx'
SHEET_NAME = 'TestPlan'


def autosize(ws):
    # Compute max length per column and set width with padding
    for col_idx, column_cells in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column), start=1):
        max_length = 0
        for cell in column_cells:
            val = cell.value
            if val is None:
                length = 0
            else:
                s = str(val)
                # Limit the measured length to avoid extremely wide columns
                length = min(len(s), 120)
            if length > max_length:
                max_length = length
        adjusted = max_length + 2
        ws.column_dimensions[get_column_letter(col_idx)].width = adjusted


def main():
    # Ensure directory exists
    OUTPUT_XLSX.parent.mkdir(parents=True, exist_ok=True)

    with open(INPUT_JSON, 'r', encoding='utf-8') as f:
        payload = json.load(f)

    # Validate JSON: must contain key 'data' as a list
    if not isinstance(payload, dict) or 'data' not in payload or not isinstance(payload['data'], list):
        raise SystemExit('Invalid input JSON: expected object with key "data" as an array')

    rows = payload['data']
    if len(rows) == 0:
        # Create empty workbook with just headers? With no rows we cannot infer columns.
        # Use default 12-column TestPlan headers.
        headers = [
            'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
            'Speed', 'Mode', 'Remarks', 'Test Steps / Procedure', 'Impacted Registers',
            'Validation / Acceptance Criteria', 'Gap Analysis'
        ]
    else:
        # Preserve the key order from the first row
        # Python 3.7+ preserves dict insertion order
        headers = list(rows[0].keys())

    wb = Workbook()
    ws = wb.active
    ws.title = SHEET_NAME

    # Write header row with bold font
    bold = Font(bold=True)
    for col_idx, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=col_idx, value=h)
        cell.font = bold

    # Write data rows preserving order
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, h in enumerate(headers, start=1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(h, ''))

    # Freeze top row
    ws.freeze_panes = 'A2'

    # Auto-size columns
    autosize(ws)

    # Save as real .xlsx
    wb.save(OUTPUT_XLSX)


if __name__ == '__main__':
    main()
