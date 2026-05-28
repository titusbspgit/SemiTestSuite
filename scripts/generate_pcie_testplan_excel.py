#!/usr/bin/env python3
import os
from openpyxl import Workbook
from openpyxl.styles import Font

# Fixed worksheet name and ordered columns per specification
SHEET_NAME = "PCIE_TestPlan"
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

# Target output path (fixed as instructed)
OUTPUT_PATH = os.path.join("Test_Output", "PCIE", "PCIE_TestPlan.xlsx")

# Embedded data (preserve exactly as provided)
DATA = [
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

def main():
    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)

    wb = Workbook()
    ws = wb.active
    ws.title = SHEET_NAME

    # Write header with bold formatting
    header_font = Font(bold=True)
    for col_idx, col_name in enumerate(COLUMNS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font

    # Write data rows, preserving column order and filling blanks for missing keys
    for row_idx, item in enumerate(DATA, start=2):
        for col_idx, col_name in enumerate(COLUMNS, start=1):
            ws.cell(row=row_idx, column=col_idx, value=item.get(col_name, ""))

    # Freeze top row
    ws.freeze_panes = "A2"

    # Save real .xlsx
    wb.save(OUTPUT_PATH)
    print(f"Wrote Excel to {OUTPUT_PATH}")

if __name__ == "__main__":
    main()
