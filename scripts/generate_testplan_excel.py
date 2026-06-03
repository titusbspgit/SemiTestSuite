#!/usr/bin/env python3
import json
import sys
import os
import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

HEADERS = [
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

def main():
    input_path = os.environ.get("TESTPLAN_JSON_PATH", "input/TestPlan.json")
    output_dir = os.environ.get("TESTPLAN_OUTPUT_DIR", "Test_Output/GPIO")

    try:
        with open(input_path, "r", encoding="utf-8") as f:
            raw = f.read()
    except FileNotFoundError:
        print(f"ERROR: Input JSON file not found at {input_path}", file=sys.stderr)
        sys.exit(1)

    try:
        data = json.loads(raw)
    except Exception as e:
        print(f"ERROR: input JSON is invalid: {e}", file=sys.stderr)
        sys.exit(1)

    if not isinstance(data, list):
        print("ERROR: input JSON must be an array of objects", file=sys.stderr)
        sys.exit(1)

    for i, row in enumerate(data):
        if not isinstance(row, dict):
            print(f"ERROR: element at index {i} is not an object", file=sys.stderr)
            sys.exit(1)

    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Header
    for col_idx, h in enumerate(HEADERS, start=1):
        c = ws.cell(row=1, column=col_idx, value=h)
        c.font = Font(bold=True)
    ws.freeze_panes = "A2"

    # Rows
    for r_idx, obj in enumerate(data, start=2):
        for c_idx, key in enumerate(HEADERS, start=1):
            val = obj.get(key, "")
            if isinstance(val, (dict, list)):
                try:
                    val = json.dumps(val, ensure_ascii=False)
                except Exception:
                    val = str(val)
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Save with IST timestamp
    ts = datetime.datetime.now(ZoneInfo("Asia/Kolkata")).strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"
    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, filename)
    wb.save(out_path)
    print(out_path)

if __name__ == "__main__":
    main()
