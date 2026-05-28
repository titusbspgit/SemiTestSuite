#!/usr/bin/env python3
import json
import os
import sys
import argparse
from datetime import datetime
try:
    from zoneinfo import ZoneInfo  # Python 3.9+
    ZI_AVAILABLE = True
except Exception:
    ZI_AVAILABLE = False
from openpyxl import Workbook
from openpyxl.styles import Font

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
    "Imparted Registers" if False else "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Gap Analysis",
]

def ist_timestamp():
    if ZI_AVAILABLE:
        tz = ZoneInfo("Asia/Kolkata")
        return datetime.now(tz).strftime("%Y%m%d_%H%M%S")
    # Fallback: IST = UTC+05:30
    from datetime import timezone, timedelta
    tz = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(tz).strftime("%Y%m%d_%H%M%S")


def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array of objects")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Each item must be an object; item {i} is {type(row)}")
    return data


def write_excel(rows, outdir, sheet_name="TestPlan"):
    os.makedirs(outdir, exist_ok=True)
    ts = ist_timestamp()
    xlsx_path = os.path.join(outdir, f"testplan_{ts}.xlsx")

    wb = Workbook()
    ws = wb.active
    ws.title = sheet_name

    # Header
    for col_idx, col_name in enumerate(COLUMNS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"

    # Rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, col_name in enumerate(COLUMNS, start=1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(col_name, ""))

    wb.save(xlsx_path)
    print(xlsx_path)
    return xlsx_path


def main():
    ap = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    ap.add_argument('--input', required=False, default='Test_Output/GPIO/testplan.json')
    ap.add_argument('--outdir', required=False, default='Test_Output/GPIO')
    ap.add_argument('--sheet-name', required=False, default='TestPlan')
    args = ap.parse_args()

    rows = load_json(args.input)
    xlsx = write_excel(rows, args.outdir, args.sheet_name)
    print(f"Wrote Excel: {xlsx}")

if __name__ == '__main__':
    main()
