#!/usr/bin/env python3
import os, json, sys
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter

def main():
    data_str = os.getenv('JSON_DATA', '').strip()
    if not data_str:
        print('ERROR: JSON_DATA env var is empty')
        sys.exit(1)
    try:
        rows = json.loads(data_str)
    except Exception as e:
        print(f'ERROR: invalid JSON: {e}')
        sys.exit(2)
    if not isinstance(rows, list):
        print('ERROR: json_data must be a JSON array of objects')
        sys.exit(3)

    # Determine headers: preserve first object's key order, then append any unseen keys
    headers = []
    if rows:
        if isinstance(rows[0], dict):
            headers = list(rows[0].keys())
        seen = set(headers)
        extras = []
        for r in rows:
            if isinstance(r, dict):
                for k in r.keys():
                    if k not in seen:
                        extras.append(k)
                        seen.add(k)
        headers += extras

    wb = Workbook()
    ws = wb.active
    ws.title = 'TestPlan'

    # Header row with bold font
    for c, h in enumerate(headers, 1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'

    # Data rows
    r_idx = 2
    for obj in rows:
        if not isinstance(obj, dict):
            obj = {}
        for c, h in enumerate(headers, 1):
            v = obj.get(h, None)
            if isinstance(v, (list, dict)):
                v = json.dumps(v, ensure_ascii=False)
            ws.cell(row=r_idx, column=c, value=v)
        r_idx += 1

    # Autosize columns (best-effort, capped)
    for c, h in enumerate(headers, 1):
        max_len = len(str(h))
        for r in range(2, r_idx):
            val = ws.cell(row=r, column=c).value
            if val is None:
                continue
            max_len = max(max_len, len(str(val)))
        ws.column_dimensions[get_column_letter(c)].width = min(80, max_len + 2)

    out_path = os.getenv('OUTPUT_PATH', 'PCIE_TestPlan.xlsx')
    out_dir = os.path.dirname(out_path)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    wb.save(out_path)

if __name__ == '__main__':
    main()
