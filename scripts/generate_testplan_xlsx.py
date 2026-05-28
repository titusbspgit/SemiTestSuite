#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate a REAL .xlsx TestPlan from a JSON array of row objects.
- Preserves all JSON fields: required columns first, then any extra columns.
- Header bold; first row frozen.
- Filename: testplan_<YYYYMMDD_HHMMSS_IST>.xlsx in the specified output directory.
"""
import argparse
import json
import os
import sys
from datetime import datetime, timedelta, timezone
from typing import List, Dict, Any

try:
    import requests  # optional when using --json-url
except Exception:
    requests = None

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment
from openpyxl.utils import get_column_letter

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
    "Imparted Registers" if False else "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Gap Analysis",
]


def to_ist_timestamp(dt: datetime = None) -> str:
    if dt is None:
        dt = datetime.utcnow().replace(tzinfo=timezone.utc)
    ist = timezone(timedelta(hours=5, minutes=30))
    ist_dt = dt.astimezone(ist)
    return ist_dt.strftime("%Y%m%d_%H%M%S")


def load_json_from_path(p: str) -> List[Dict[str, Any]]:
    with open(p, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("JSON root must be an array of objects")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Each array element must be an object; element {i} is {type(row)}")
    return data


def load_json_from_url(url: str) -> List[Dict[str, Any]]:
    if requests is None:
        raise RuntimeError("requests not available; cannot use --json-url")
    r = requests.get(url, timeout=60)
    r.raise_for_status()
    data = r.json()
    if not isinstance(data, list):
        raise ValueError("JSON root must be an array of objects")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Each array element must be an object; element {i} is {type(row)}")
    return data


def normalize_value(v: Any) -> Any:
    if v is None:
        return None
    if isinstance(v, (str, int, float)):
        return v
    # Preserve complex data as JSON string
    try:
        return json.dumps(v, ensure_ascii=False)
    except Exception:
        return str(v)


def compute_columns(rows: List[Dict[str, Any]]) -> List[str]:
    # All unique keys from data
    keys = []
    seen = set()
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.add(k)
                keys.append(k)
    # Keep required first in given order, then any extras in the order discovered
    extras = [k for k in keys if k not in REQUIRED_COLUMNS]
    return REQUIRED_COLUMNS + extras


def autosize_columns(ws):
    widths = {}
    for row in ws.iter_rows(values_only=True):
        for i, cell in enumerate(row, start=1):
            val = "" if cell is None else str(cell)
            w = len(val)
            if w > widths.get(i, 0):
                widths[i] = w
    for i, w in widths.items():
        # Add padding and cap width
        adj = min(w + 2, 80)
        ws.column_dimensions[get_column_letter(i)].width = adj


def write_excel(rows: List[Dict[str, Any]], out_dir: str) -> str:
    os.makedirs(out_dir, exist_ok=True)
    ts = to_ist_timestamp()
    filename = f"testplan_{ts}.xlsx"
    out_path = os.path.join(out_dir, filename)

    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    columns = compute_columns(rows)

    # Header
    for col_idx, name in enumerate(columns, start=1):
        cell = ws.cell(row=1, column=col_idx, value=name)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(wrap_text=True, vertical="top")

    # Data rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, col in enumerate(columns, start=1):
            val = normalize_value(row.get(col))
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Freeze header
    ws.freeze_panes = "A2"

    # Basic auto-size
    autosize_columns(ws)

    wb.save(out_path)
    return out_path


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON array of row objects")
    parser.add_argument("--json-path", help="Path to local JSON file (array of objects)")
    parser.add_argument("--json-url", help="URL to JSON (array of objects)")
    parser.add_argument("--output-dir", required=True, help="Directory for output .xlsx (will be created if needed)")
    args = parser.parse_args()

    if not args.json_path and not args.json_url:
        print("ERROR: Provide either --json-path or --json-url", file=sys.stderr)
        sys.exit(1)

    try:
        if args.json_path:
            data = load_json_from_path(args.json_path)
        else:
            data = load_json_from_url(args.json_url)
    except Exception as e:
        print(f"ERROR: Failed to load JSON: {e}", file=sys.stderr)
        sys.exit(2)

    try:
        out_path = write_excel(data, args.output_dir)
    except Exception as e:
        print(f"ERROR: Failed to write Excel: {e}", file=sys.stderr)
        sys.exit(3)

    print(out_path)


if __name__ == "__main__":
    main()
