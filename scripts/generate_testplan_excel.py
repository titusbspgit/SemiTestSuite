#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate a real Excel (.xlsx) TestPlan sheet from consolidated JSON.

Usage:
  python scripts/generate_testplan_excel.py --input testplan_input.json --output-dir Test_Output/GPIO [--ip-name GPIO]

Rules implemented:
- Sheet name: TestPlan
- Columns (in order):
  Index, SS / Module, Feature, Test Case Name, Test Description, Speed, Mode, Remarks,
  Test Steps / Procedure, Impacted Registers, Validation / Acceptance Criteria, Gap Analysis
- Header bold, freeze first row
- Filename: testplan_<YYYYMMDD_HHMMSS_IST>.xlsx (Asia/Kolkata)
- Data mapping:
  * Index: 1..n
  * SS / Module: ip_name (from CLI or JSON top-level)
  * Test Case Name: testcase.name or testcase.id
  * Test Description: testcase.description if present, else blank
  * Test Steps / Procedure: join(testcase.steps) if present
  * Impacted Registers: join(testcase.impacted_registers) if present
  * Validation / Acceptance Criteria: testcase.expected_result or testcase.acceptance_criteria if present
  * Remarks: packs unmapped but important fields (id, file_path, source_github_url, tags) to avoid data loss

This script requires openpyxl.
"""

import argparse
import json
import sys
from pathlib import Path
from datetime import datetime

try:
    from zoneinfo import ZoneInfo  # Python 3.9+
except Exception:  # pragma: no cover
    ZoneInfo = None

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
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Gap Analysis",
]


def load_rows_and_meta(input_path: Path, override_ip_name: str | None):
    with input_path.open("r", encoding="utf-8") as f:
        data = json.load(f)

    ip_name = override_ip_name

    if isinstance(data, list):
        rows = data
        # try pick ip_name from any row if present
        if ip_name is None:
            for r in rows:
                if isinstance(r, dict) and r.get("ip_name"):
                    ip_name = r["ip_name"]
                    break
    elif isinstance(data, dict):
        rows = data.get("testcases", [])
        if ip_name is None:
            ip_name = data.get("ip_name")
    else:
        raise ValueError("Input JSON must be either an array or an object with key 'testcases'.")

    if not isinstance(rows, list):
        raise ValueError("'testcases' must be an array when using object input JSON.")

    if ip_name is None:
        ip_name = ""  # leave blank if not provided

    return rows, ip_name


def to_str_list(val):
    if val is None:
        return []
    if isinstance(val, list):
        return [str(x) for x in val]
    return [str(val)]


def build_excel(rows, ip_name: str, out_dir: Path) -> Path:
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Header
    ws.append(COLUMNS)
    for cell in ws[1]:
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"

    # Populate rows
    for idx, tc in enumerate(rows, start=1):
        if not isinstance(tc, dict):
            # skip invalid row types but keep index continuity
            continue
        name = tc.get("name") or tc.get("id") or ""
        desc = tc.get("description", "")
        steps = to_str_list(tc.get("steps"))
        steps_joined = " | ".join(steps)
        impacted_regs = to_str_list(tc.get("impacted_registers"))
        impacted_regs_joined = ", ".join(impacted_regs)
        vac = tc.get("expected_result") or tc.get("acceptance_criteria") or ""

        remarks_parts = []
        if tc.get("id"):
            remarks_parts.append(f"id={tc['id']}")
        if tc.get("file_path"):
            remarks_parts.append(f"file_path={tc['file_path']}")
        if tc.get("source_github_url"):
            remarks_parts.append(f"source={tc['source_github_url']}")
        if tc.get("tags"):
            try:
                remarks_parts.append("tags=" + ",".join([str(t) for t in tc.get("tags", [])]))
            except Exception:
                pass
        if tc.get("framework"):
            remarks_parts.append(f"framework={tc['framework']}")
        remarks = " | ".join(remarks_parts)

        row = [
            idx,                    # Index
            ip_name or "",         # SS / Module
            "",                    # Feature
            name,                   # Test Case Name
            desc,                   # Test Description
            "",                    # Speed
            "",                    # Mode
            remarks,                # Remarks
            steps_joined,           # Test Steps / Procedure
            impacted_regs_joined,   # Impacted Registers
            vac,                    # Validation / Acceptance Criteria
            "",                    # Gap Analysis
        ]
        ws.append(row)

    # Best-effort column width
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            try:
                val = str(cell.value) if cell.value is not None else ""
            except Exception:
                val = ""
            max_len = max(max_len, len(val))
        ws.column_dimensions[col_letter].width = min(max(10, max_len + 2), 60)

    # Compute IST timestamp
    if ZoneInfo is not None:
        now_ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    else:
        # Fallback to naive localtime labeled as IST (may differ on runner)
        now_ist = datetime.now()
    ts = now_ist.strftime("%Y%m%d_%H%M%S")

    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"testplan_{ts}.xlsx"
    wb.save(str(out_path))
    print(f"Wrote Excel: {out_path}")
    return out_path


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    parser.add_argument("--input", required=True, help="Path to consolidated JSON input")
    parser.add_argument("--output-dir", required=True, help="Directory to save Excel file into")
    parser.add_argument("--ip-name", default=None, help="Override ip_name for 'SS / Module' column")
    args = parser.parse_args()

    input_path = Path(args.input)
    output_dir = Path(args.output_dir)

    rows, ip_name = load_rows_and_meta(input_path, args.ip_name)
    # Validate rows is a list (array)
    if not isinstance(rows, list):
        print("ERROR: json_data must be an array or contain key 'testcases' as an array.", file=sys.stderr)
        sys.exit(1)

    build_excel(rows, ip_name or "", output_dir)


if __name__ == "__main__":
    main()
