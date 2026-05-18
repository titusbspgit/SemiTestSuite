#!/usr/bin/env python3
"""
Generate a REAL .xlsx Test Plan from JSON using openpyxl.
- Validates input JSON as an array of objects (one row per object).
- Creates a workbook with a single sheet named 'TestPlan'.
- Writes fixed headers in the required order, bolds the header row, and freezes the first row.
- Preserves all provided values for the specified columns without transformation.
- Saves as testplan_<YYYYMMDD_HHMMSS>.xlsx in the specified output directory.

Usage:
  python generate_testplan_excel.py --input <path/to/testplan.json> --output-dir <output/dir> [--timestamp YYYYMMDD_HHMMSS]

Exit codes:
  0 on success
  1 on validation or runtime error
"""

import argparse
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from zoneinfo import ZoneInfo  # Python 3.9+
except Exception:
    ZoneInfo = None

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font
except ImportError as e:
    print(f"ERROR: openpyxl is required: {e}", file=sys.stderr)
    sys.exit(1)

REQUIRED_HEADERS = [
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


def parse_args():
    p = argparse.ArgumentParser(description="Convert Test Plan JSON to Excel (.xlsx)")
    p.add_argument("--input", required=True, help="Path to input JSON file")
    p.add_argument("--output-dir", required=True, help="Directory to write the Excel file to")
    p.add_argument("--timestamp", required=False, help="Timestamp in IST, format YYYYMMDD_HHMMSS")
    return p.parse_args()


def load_json(path):
    try:
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
    except Exception as e:
        raise RuntimeError(f"Failed to read JSON: {e}")

    if not isinstance(data, list):
        raise RuntimeError("Input JSON must be an array (list) of row objects")

    for i, row in enumerate(data, start=1):
        if not isinstance(row, dict):
            raise RuntimeError(f"Element at index {i-1} is not an object (dict)")
    return data


def compute_ist_timestamp(ts_arg=None):
    fmt = "%Y%m%d_%H%M%S"
    if ts_arg:
        # Basic sanity check of provided timestamp
        try:
            datetime.strptime(ts_arg, fmt)
        except ValueError:
            raise RuntimeError("--timestamp must be in format YYYYMMDD_HHMMSS")
        return ts_arg
    # Compute now in IST
    if ZoneInfo is not None:
        tz = ZoneInfo("Asia/Kolkata")
        now = datetime.now(tz)
    else:
        # Fallback fixed-offset IST (UTC+5:30)
        tz = timezone(timedelta(hours=5, minutes=30))
        now = datetime.now(tz)
    return now.strftime(fmt)


def build_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Header
    ws.append(REQUIRED_HEADERS)
    for cell in ws[1]:
        cell.font = Font(bold=True)

    # Freeze first row
    ws.freeze_panes = "A2"

    # Data rows
    for row in rows:
        values = []
        for key in REQUIRED_HEADERS:
            val = row.get(key, "")
            if val is None:
                val = ""
            # Ensure str for non-str types without altering content for strings
            if isinstance(val, str):
                values.append(val)
            else:
                values.append(str(val))
        ws.append(values)

    return wb


def main():
    args = parse_args()

    rows = load_json(args.input)

    # Optional: ensure all required headers exist in each row (fill missing as empty)
    # Strict validation: warn if any required header missing
    missing_any = False
    for i, r in enumerate(rows, start=1):
        for h in REQUIRED_HEADERS:
            if h not in r:
                missing_any = True
                print(f"WARNING: Row {i} missing column '{h}', filling with empty string", file=sys.stderr)
    if missing_any:
        # Not a hard failure per requirements; we preserve all available data.
        pass

    ts = compute_ist_timestamp(args.timestamp)

    wb = build_workbook(rows)

    # Ensure output directory exists
    out_dir = os.path.abspath(args.output_dir)
    os.makedirs(out_dir, exist_ok=True)

    out_path = os.path.join(out_dir, f"testplan_{ts}.xlsx")
    try:
        wb.save(out_path)
    except Exception as e:
        raise RuntimeError(f"Failed to save Excel file: {e}")

    print(out_path)


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
