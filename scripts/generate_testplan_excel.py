#!/usr/bin/env python3
import argparse, json, os
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    from backports.zoneinfo import ZoneInfo
import pandas as pd
from openpyxl.styles import Font

REQUIRED_COLUMNS = [
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
    ap = argparse.ArgumentParser()
    ap.add_argument('--json-path', required=True)
    ap.add_argument('--output-dir', required=True)
    args = ap.parse_args()

    with open(args.json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    if not isinstance(data, list):
        raise ValueError('json_data must be an array of objects')
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f'Row {i} is not an object')

    # Build rows ensuring exact column order; fill missing keys with ''
    rows = []
    for row in data:
        rows.append({col: row.get(col, '') for col in REQUIRED_COLUMNS})

    df = pd.DataFrame(rows, columns=REQUIRED_COLUMNS, dtype=object)

    ist = ZoneInfo('Asia/Kolkata')
    ts = datetime.now(ist).strftime('%Y%m%d_%H%M%S')
    filename = f'testplan_{ts}.xlsx'
    os.makedirs(args.output_dir, exist_ok=True)
    xlsx_path = os.path.join(args.output_dir, filename)

    # Write with pandas (openpyxl engine), then format header and freeze
    with pd.ExcelWriter(xlsx_path, engine='openpyxl') as writer:
        df.to_excel(writer, index=False, sheet_name='TestPlan')
        ws = writer.book['TestPlan']
        for cell in ws[1]:
            cell.font = Font(bold=True)
        ws.freeze_panes = 'A2'

    print(xlsx_path)

if __name__ == '__main__':
    main()
