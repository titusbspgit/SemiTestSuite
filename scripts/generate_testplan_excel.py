#!/usr/bin/env python3
# Ag-Emb-Testsuite-Excel-Generator Agent fallback script
# Purpose: Convert embedded Test Plan JSON into a REAL .xlsx (not CSV) and save to the specified output directory.
# Implements:
# - Single sheet named by SHEET_NAME (default: "TestPlan")
# - Bold header row
# - Freeze first row
# - Filename pattern: testplan_<YYYYMMDD_HHMMSS>.xlsx (IST time)

from __future__ import annotations
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
import os
from typing import List, Dict, Any

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font
except Exception as e:
    raise SystemExit(f"openpyxl is required. Install with: pip install openpyxl. Error: {e}")

# ========= Embedded JSON data (exact, no modification) =========
JSON_DATA: List[Dict[str, Any]] = [
    {
      "Index": "1",
      "SS / Module": "PCIE",
      "Feature": "pcie_cfg_wr_rd_test",
      "Test Case Name": "pcie_cfg_wr_rd_test",
      "Test Description": "NA",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "NA",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "2",
      "SS / Module": "PCIE",
      "Feature": "pcie_dbi_dsp_reg_wr_rd_test",
      "Test Case Name": "pcie_dbi_dsp_reg_wr_rd_test",
      "Test Description": "NA",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "NA",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "3",
      "SS / Module": "PCIE",
      "Feature": "pcie_dbi_usp_reg_wr_rd_test",
      "Test Case Name": "pcie_dbi_usp_reg_wr_rd_test",
      "Test Description": "NA",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "NA",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    },
    {
      "Index": "4",
      "SS / Module": "PCIE",
      "Feature": "pcie_sii_rc_reg_wr_rd_test",
      "Test Case Name": "pcie_sii_rc_reg_wr_rd_test",
      "Test Description": "NA",
      "Speed": "NA",
      "Mode": "NA",
      "Remarks": "NA",
      "Test Steps / Procedure": "NA",
      "Impacted Registers": "NA",
      "Validation / Acceptance Criteria": "NA",
      "Gap Analysis": "NA"
    }
]
# ===============================================================

# Required column order
COLUMNS: List[str] = [
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


def main() -> None:
    # STEP 1 — Validate JSON
    if not isinstance(JSON_DATA, list):
        raise SystemExit("json_data must be an array of objects")
    for i, row in enumerate(JSON_DATA, start=1):
        if not isinstance(row, dict):
            raise SystemExit(f"Each item must be an object. Invalid at index {i-1}")

    # STEP 2 — Create workbook and sheet
    wb = Workbook()
    ws = wb.active
    sheet_name = os.getenv("SHEET_NAME", "TestPlan")
    ws.title = sheet_name

    # Header row with bold font
    bold_font = Font(bold=True)
    for col_idx, header in enumerate(COLUMNS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=header)
        cell.font = bold_font

    # Freeze first row
    ws.freeze_panes = "A2"

    # STEP 3 — Write rows
    row_idx = 2
    for row in JSON_DATA:
        for col_idx, header in enumerate(COLUMNS, start=1):
            value = row.get(header, None)
            ws.cell(row=row_idx, column=col_idx, value=value)
        row_idx += 1

    # Ensure output directory exists
    output_dir = os.getenv("OUTPUT_DIR", "Test_Output/GPIO")
    out_path = Path(output_dir)
    out_path.mkdir(parents=True, exist_ok=True)

    # Filename with IST timestamp
    ist_now = datetime.now(ZoneInfo("Asia/Kolkata"))
    ts = ist_now.strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"
    fullpath = out_path / filename

    # Save as REAL .xlsx
    wb.save(fullpath)

    # Print path for CI logs/consumers
    print(str(fullpath))


if __name__ == "__main__":
    main()
