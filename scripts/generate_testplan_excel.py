#!/usr/bin/env python3
import argparse, json, os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font


def to_ist_timestamp():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime('%Y%m%d_%H%M%S')


def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError('json_data must be an array of objects')
    for i, row in enumerate(data, 1):
        if not isinstance(row, dict):
            raise ValueError(f'Row {i} is not an object')
    return data


def build_workbook(rows, sheet_name):
    wb = Workbook()
    ws = wb.active
    ws.title = sheet_name

    # Determine column order from the first row keys (preserve provided order)
    columns = list(rows[0].keys()) if rows else []

    # Header
    bold = Font(bold=True)
    for c, col in enumerate(columns, start=1):
        cell = ws.cell(row=1, column=c, value=col)
        cell.font = bold
    ws.freeze_panes = 'A2'

    # Rows
    for r, row in enumerate(rows, start=2):
        for c, col in enumerate(columns, start=1):
            ws.cell(row=r, column=c, value=row.get(col, ''))

    return wb


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--input', required=True, help='Path to input JSON array file')
    ap.add_argument('--outdir', required=True, help='Output directory for Excel file')
    ap.add_argument('--sheet', default='PCIE_TestPlan', help='Sheet name (default: PCIE_TestPlan)')
    args = ap.parse_args()

    rows = load_json(args.input)
    if not rows:
        # still create a header-only sheet if empty
        rows = [{
            "Index": "",
            "SS / Module": "",
            "Feature": "",
            "Test Case Name": "",
            "Test Description": "",
            "Speed": "",
            "Mode": "",
            "Remarks": "",
            "Test Steps / Procedure": "",
            "Impacted Registers": "",
            "Validation / Acceptance Criteria": "",
            "Gap Analysis": ""
        }]

    wb = build_workbook(rows, args.sheet)

    os.makedirs(args.outdir, exist_ok=True)
    # Compute IST timestamp (YYYYMMDD_HHMMSS)
    ts = to_ist_timestamp()
    out_path = os.path.join(args.outdir, f'testplan_{ts}.xlsx')
    wb.save(out_path)

    # Output path for logs
    print(out_path)


if __name__ == '__main__':
    main()
