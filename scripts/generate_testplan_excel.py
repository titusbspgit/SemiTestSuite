#!/usr/bin/env python3
import json
import os
from datetime import datetime, timedelta, timezone
from pathlib import Path

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment

ROOT = Path(__file__).resolve().parents[1]
DATA_JSON_PATH = ROOT / 'data' / 'testplan_gpio.json'
OUTPUT_DIR = Path(os.environ.get('OUTPUT_DIR', ROOT / 'Test_Output' / 'GPIO'))
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

# IST timezone (UTC+5:30)
IST = timezone(timedelta(hours=5, minutes=30))

def ist_timestamp():
    return datetime.now(IST).strftime('%Y%m%d_%H%M%S')

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
    'Gap Analysis',
]


def load_json():
    with open(DATA_JSON_PATH, 'r', encoding='utf-8') as f:
        data = json.load(f)
    # Accept either an array of tests or an object with 'tests'
    if isinstance(data, list):
        top_level = {}
        tests = data
    elif isinstance(data, dict) and 'tests' in data and isinstance(data['tests'], list):
        top_level = data
        tests = data['tests']
    else:
        raise ValueError('Invalid json_data: expected array or object with key "tests" as array')
    return top_level, tests


def to_json_str(obj):
    return json.dumps(obj, ensure_ascii=False, separators=(',', ':'), sort_keys=False)


def build_rows(top_level, tests):
    ip = top_level.get('ip', '') if isinstance(top_level, dict) else ''
    rows = []
    for idx, t in enumerate(tests, start=1):
        feature = t.get('type') or ','.join(t.get('tags', []) or [])
        test_name = t.get('id') or t.get('name') or ''
        description = t.get('description') or ''
        steps = '\n'.join(t.get('steps', []) or [])
        impacted_regs = to_json_str(((t.get('parameters') or {}).get('registers') or {}))
        vac = '\n'.join(t.get('expected_results', []) or [])
        remarks = to_json_str(t)
        row = [
            idx,                 # Index
            ip,                  # SS / Module
            feature or '',       # Feature
            test_name,           # Test Case Name
            description,         # Test Description
            '',                  # Speed (unknown)
            '',                  # Mode (unknown)
            remarks,             # Remarks (full test JSON to avoid data loss)
            steps,               # Test Steps / Procedure
            impacted_regs,       # Impacted Registers
            vac,                 # Validation / Acceptance Criteria
            '',                  # Gap Analysis
        ]
        rows.append(row)
    return rows


def write_excel(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = 'TestPlan'

    # Header
    ws.append(HEADERS)
    for cell in ws[1]:
        cell.font = Font(bold=True)
        cell.alignment = Alignment(vertical='center')

    # Data rows
    wrap_cols = {8, 9, 10, 11}  # Remarks, Steps, Impacted Registers, VAC
    for row in rows:
        ws.append(row)
    # Apply wrap and top alignment
    for r in ws.iter_rows(min_row=2, max_row=ws.max_row):
        for i, cell in enumerate(r, start=1):
            if i in wrap_cols:
                cell.alignment = Alignment(wrap_text=True, vertical='top')
            else:
                cell.alignment = Alignment(vertical='top')

    # Freeze header row
    ws.freeze_panes = 'A2'

    # Best-effort column width
    for i, header in enumerate(HEADERS, start=1):
        if header in ('Remarks', 'Test Steps / Procedure', 'Validation / Acceptance Criteria'):
            ws.column_dimensions[chr(64+i)].width = 60
        elif header in ('Impacted Registers', 'Feature', 'Test Description'):
            ws.column_dimensions[chr(64+i)].width = 30
        else:
            ws.column_dimensions[chr(64+i)].width = 18

    ts = ist_timestamp()
    out_path = OUTPUT_DIR / f'testplan_{ts}.xlsx'
    wb.save(out_path)
    print(f'Wrote Excel: {out_path}')
    return str(out_path)


def main():
    top_level, tests = load_json()
    rows = build_rows(top_level, tests)
    path = write_excel(rows)
    # Also drop a pointer file to the latest path (optional, non-fatal)
    try:
        with open(OUTPUT_DIR / 'LATEST_PATH.txt', 'w', encoding='utf-8') as f:
            f.write(path + '\n')
    except Exception:
        pass

if __name__ == '__main__':
    main()
