#!/usr/bin/env python3
# Ag-Emb-Testsuite-Excel-Generator Agent: JSON -> Excel generator (Fallback)

import json
import os
import sys
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Convert Test Plan JSON to Excel (.xlsx)')
    parser.add_argument('--input', default='testplan_input.json', help='Path to input JSON file')
    parser.add_argument('--output-dir', required=True, help='Directory to place generated Excel file')
    args = parser.parse_args()

    # STEP 1 — Validate JSON
    with open(args.input, 'r', encoding='utf-8') as f:
        data = json.load(f)

    ip_name = ''
    rows = None

    if isinstance(data, dict):
        ip_name = data.get('ip_name') or data.get('IP_NAME') or ''
        rows = data.get('testcases') if isinstance(data.get('testcases'), list) else None
        if rows is None and isinstance(data.get('data'), list):
            rows = data.get('data')
    elif isinstance(data, list):
        rows = data
    else:
        raise ValueError('json_data must be an object with a "testcases" array or a list of rows')

    if not isinstance(rows, list):
        raise ValueError('json_data.testcases must be an array')

    # STEP 2 — Create .xlsx workbook with formatting
    wb = Workbook()
    ws = wb.active
    ws.title = 'TestPlan'

    headers = [
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

    ws.append(headers)
    for cell in ws[1]:
        cell.font = Font(bold=True)

    for tc in rows:
        index = tc.get('index', '')
        case_name = tc.get('test_name') or tc.get('folder_name') or ''
        desc = tc.get('objective') or ''

        github_url = tc.get('github_url') or ''
        raw_base_url = tc.get('raw_base_url') or ''
        remarks_parts = []
        if github_url:
            remarks_parts.append(f'github_url={github_url}')
        if raw_base_url:
            remarks_parts.append(f'raw_base_url={raw_base_url}')
        remarks = '; '.join(remarks_parts)

        row = [
            index,               # Index
            ip_name,             # SS / Module
            '',                  # Feature
            case_name,           # Test Case Name
            desc,                # Test Description
            '',                  # Speed
            '',                  # Mode
            remarks,             # Remarks
            '',                  # Test Steps / Procedure
            '',                  # Impacted Registers
            '',                  # Validation / Acceptance Criteria
            '',                  # Gap Analysis
        ]
        ws.append(row)

    # Freeze first row
    ws.freeze_panes = 'A2'

    # STEP 3 — Save File with IST timestamp
    ist = ZoneInfo('Asia/Kolkata')
    ts = datetime.now(tz=ist).strftime('%Y%m%d_%H%M%S')
    filename = f'testplan_{ts}.xlsx'

    os.makedirs(args.output_dir, exist_ok=True)
    out_path = os.path.join(args.output_dir, filename)
    wb.save(out_path)

    # Print the generated path for workflow logs
    print(out_path)


if __name__ == '__main__':
    main()
