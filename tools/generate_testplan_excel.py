#!/usr/bin/env python3
import json
import sys
import os
from pathlib import Path
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None

from openpyxl import Workbook
from openpyxl.styles import Font

REQUIRED_COLUMNS = [
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
]


def load_json(json_path: Path):
    with open(json_path, 'r', encoding='utf-8') as f:
        obj = json.load(f)
    # Accept either a list directly or an object with key 'data'
    if isinstance(obj, list):
        data = obj
    elif isinstance(obj, dict) and isinstance(obj.get('data'), list):
        data = obj['data']
    else:
        raise ValueError("Invalid json_data: expected array or object with key 'data' as array")
    # Validate rows are objects
    for i, row in enumerate(data, start=1):
        if not isinstance(row, dict):
            raise ValueError(f"Invalid row at index {i}: expected object, got {type(row)}")
    return data


def autosize_columns(ws):
    for col_cells in ws.columns:
        max_length = 0
        col_letter = col_cells[0].column_letter
        for cell in col_cells:
            try:
                val = "" if cell.value is None else str(cell.value)
            except Exception:
                val = ""
            if len(val) > max_length:
                max_length = len(val)
        # add padding
        ws.column_dimensions[col_letter].width = min(80, max(10, max_length + 2))


def make_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Header
    header_font = Font(bold=True)
    ws.append(REQUIRED_COLUMNS)
    for cell in ws[1]:
        cell.font = header_font
    ws.freeze_panes = "A2"

    # Rows in required order; fill missing keys with empty string
    for r in rows:
        ws.append([r.get(col, "") for col in REQUIRED_COLUMNS])

    autosize_columns(ws)
    return wb


def ensure_dir(p: Path):
    p.mkdir(parents=True, exist_ok=True)


def main():
    json_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path('Test_Output/GPIO/testplan_input.json')
    out_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path('Test_Output/GPIO')

    rows = load_json(json_path)

    wb = make_workbook(rows)

    # Timestamp in IST
    if ZoneInfo is not None:
        now_ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    else:
        # Fallback to UTC if zoneinfo unavailable (rare on GH runners); label remains deterministic format
        now_ist = datetime.utcnow()
    ts = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"

    ensure_dir(out_dir)
    out_path = out_dir / filename
    wb.save(out_path)
    print(f"Wrote {out_path}")


if __name__ == '__main__':
    main()
