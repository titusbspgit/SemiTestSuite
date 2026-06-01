#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate a REAL .xlsx TestPlan from JSON using openpyxl.
- Reads: Test_Output/PCIE/testplan_input.json
- Writes: Test_Output/PCIE/testplan_<YYYYMMDD_HHMMSS IST>.xlsx
- Sheet name: PCIE_TestPlan
- Columns (exact order):
  Index, SS / Module, Feature, Test Case Name, Test Description, Speed,
  Mode, Remarks, Test Steps / Procedure, Impacted Registers,
  Validation / Acceptance Criteria, Gap Analysis

This script preserves the full original testcase object in the Remarks column
(to avoid data loss) while still mapping key fields to designated columns.
"""
import json
import os
import sys
from datetime import datetime
try:
    # Python 3.9+: zoneinfo is available; ensure tzdata exists on runner
    from zoneinfo import ZoneInfo  # type: ignore
    def ist_now_str():
        return datetime.now(ZoneInfo("Asia/Kolkata")).strftime("%Y%m%d_%H%M%S")
except Exception:
    # Fallback to naive time + 5:30 offset if zoneinfo not available
    from datetime import timedelta
    def ist_now_str():
        return (datetime.utcnow() + timedelta(hours=5, minutes=30)).strftime("%Y%m%d_%H%M%S")

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font
except Exception as e:
    print("ERROR: openpyxl not available. Install with: pip install openpyxl", file=sys.stderr)
    raise

INPUT_JSON_PATH = os.path.join("Test_Output", "PCIE", "testplan_input.json")
OUTPUT_DIR = os.path.join("Test_Output", "PCIE")
SHEET_NAME = "PCIE_TestPlan"

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
    "Imparted Registers".replace("Imparted", "Impacted"),  # ensure exact label
    "Validation / Acceptance Criteria",
    "Gap Analysis",
]


def load_json(path: str):
    if not os.path.exists(path):
        raise FileNotFoundError(f"Input JSON not found at {path}")
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    # Accept either a list or an object with key 'testcases'
    if isinstance(data, dict) and "testcases" in data and isinstance(data["testcases"], list):
        rows = data["testcases"]
    elif isinstance(data, list):
        rows = data
    else:
        raise ValueError("json_data must be an array or an object with key 'testcases' as an array")
    return rows


def as_text(x):
    if x is None:
        return "NA"
    if isinstance(x, (list, tuple)):
        return "\n".join(str(i) for i in x)
    if isinstance(x, (dict,)):
        return json.dumps(x, ensure_ascii=False)
    return str(x)


def build_rows(testcases):
    rows = []
    for i, tc in enumerate(testcases, start=1):
        component = (tc.get("component") or "NA") if isinstance(tc, dict) else "NA"
        suite = (tc.get("suite") or "NA") if isinstance(tc, dict) else "NA"
        name = (tc.get("name") or "NA") if isinstance(tc, dict) else "NA"
        description = (tc.get("description") or "NA") if isinstance(tc, dict) else "NA"
        steps_raw = []
        if isinstance(tc, dict):
            s = tc.get("steps")
            if isinstance(s, list):
                steps_raw = s
            elif s is not None:
                steps_raw = [str(s)]
        steps_text = "\n".join(str(s) for s in steps_raw) if steps_raw else "NA"
        inputs = tc.get("inputs", {}) if isinstance(tc, dict) else {}
        impacted_regs = "NA"
        if isinstance(inputs, dict):
            names = inputs.get("addr_names")
            if isinstance(names, list) and names:
                impacted_regs = ", ".join(str(n) for n in names)
        expected = (tc.get("expected_result") or "NA") if isinstance(tc, dict) else "NA"
        remarks = json.dumps(tc, ensure_ascii=False) if isinstance(tc, dict) else as_text(tc)
        row = [
            i,                  # Index
            component,          # SS / Module
            suite,              # Feature
            name,               # Test Case Name
            description,        # Test Description
            "NA",              # Speed
            "NA",              # Mode
            remarks,            # Remarks
            steps_text,         # Test Steps / Procedure
            impacted_regs,      # Impacted Registers
            expected,           # Validation / Acceptance Criteria
            "NA",              # Gap Analysis
        ]
        rows.append(row)
    return rows


def autofit(ws):
    # Compute max width per column based on cell values
    for col in ws.columns:
        max_length = 0
        col_letter = col[0].column_letter
        for cell in col:
            try:
                v = "" if cell.value is None else str(cell.value)
            except Exception:
                v = ""
            if v is None:
                v = ""
            max_length = max(max_length, len(v))
        # Reasonable width cap
        width = min(120, max(12, max_length + 2))
        ws.column_dimensions[col_letter].width = width


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    testcases = load_json(INPUT_JSON_PATH)
    # Validate array
    if not isinstance(testcases, list):
        raise ValueError("json_data must be an array or object.testcases must be an array")

    wb = Workbook()
    ws = wb.active
    ws.title = SHEET_NAME

    # Write header with bold font
    ws.append(HEADERS)
    for cell in ws[1]:
        cell.font = Font(bold=True)

    # Write rows
    for row in build_rows(testcases):
        ws.append(row)

    # Freeze top row
    ws.freeze_panes = "A2"

    # Auto-fit columns
    autofit(ws)

    # Filename with IST timestamp
    ts = ist_now_str()
    out_path = os.path.join(OUTPUT_DIR, f"testplan_{ts}.xlsx")
    wb.save(out_path)
    print(f"WROTE {out_path}")


if __name__ == "__main__":
    main()
