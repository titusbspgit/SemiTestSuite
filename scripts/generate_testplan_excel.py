#!/usr/bin/env python3
import argparse
import json
import os
import sys
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:  # Python <3.9 fallback if needed
    ZoneInfo = None

from openpyxl import Workbook
from openpyxl.styles import Font

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
    "Imparted Registers" if False else "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Gap Analysis",
]

# Build normalized mapping for header detection
import re

def normalize_key(s: str) -> str:
    return re.sub(r"[^a-z0-9]", "", s.lower())

NORMALIZED_TO_HEADER = {normalize_key(h): h for h in REQUIRED_HEADERS}


def to_cell_value(v):
    if v is None:
        return ""
    if isinstance(v, (str, int, float)):
        return v
    if isinstance(v, bool):
        return str(v)
    # For lists/dicts/others, store JSON to preserve data exactly
    try:
        return json.dumps(v, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
    except Exception:
        return str(v)


def now_ist_timestamp():
    if ZoneInfo is not None:
        ts = datetime.now(ZoneInfo("Asia/Kolkata"))
    else:
        # Fallback: IST = UTC+5:30 (naive)
        from datetime import timedelta, timezone
        ts = datetime.now(timezone(timedelta(hours=5, minutes=30)))
    return ts.strftime("%Y%m%d_%H%M%S")


def validate_json(rows):
    if not isinstance(rows, list):
        raise ValueError("json_data must be a JSON array (list of objects).")
    for i, r in enumerate(rows):
        if not isinstance(r, dict):
            raise ValueError(f"Each element must be an object (dict). Found {type(r)} at index {i}.")


def build_row(item: dict) -> dict:
    # Initialize row with blanks
    row = {h: "" for h in REQUIRED_HEADERS}

    extras = {}
    for k, v in item.items():
        nk = normalize_key(str(k))
        header = NORMALIZED_TO_HEADER.get(nk)
        if header is not None:
            if row[header] in ("", None):
                row[header] = to_cell_value(v)
            else:
                # If duplicate mapping, append as JSON to Remarks to avoid loss
                extras[k] = v
        else:
            extras[k] = v

    if extras:
        extras_json = json.dumps(extras, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
        if row["Remarks"]:
            row["Remarks"] = f"{row['Remarks']} | Extras={extras_json}"
        else:
            row["Remarks"] = f"Extras={extras_json}"

    return row


def write_excel(rows: list, output_dir: str) -> str:
    os.makedirs(output_dir, exist_ok=True)

    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Write header
    ws.append(REQUIRED_HEADERS)
    for cell in ws[1]:
        cell.font = Font(bold=True)

    # Freeze first row
    ws.freeze_panes = "A2"

    # Write data rows
    for item in rows:
        row = build_row(item)
        ws.append([row[h] for h in REQUIRED_HEADERS])

    ts = now_ist_timestamp()
    filename = f"testplan_{ts}.xlsx"
    out_path = os.path.join(output_dir, filename)
    wb.save(out_path)
    return out_path


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON using openpyxl.")
    parser.add_argument("--input", required=True, help="Path to input JSON file (array of objects)")
    parser.add_argument("--output-dir", default="Test_Output/GPIO", help="Directory to place the generated Excel")
    args = parser.parse_args()

    # Read and validate JSON
    try:
        with open(args.input, "r", encoding="utf-8") as f:
            data_raw = f.read()
        data = json.loads(data_raw)
    except Exception as e:
        print(f"ERROR: Failed to read/parse JSON: {e}", file=sys.stderr)
        sys.exit(1)

    try:
        validate_json(data)
    except Exception as e:
        print(f"ERROR: Invalid JSON structure: {e}", file=sys.stderr)
        sys.exit(1)

    try:
        out_path = write_excel(data, args.output_dir)
    except Exception as e:
        print(f"ERROR: Failed to write Excel: {e}", file=sys.stderr)
        sys.exit(1)

    print(out_path)


if __name__ == "__main__":
    main()
