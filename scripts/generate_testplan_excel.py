#!/usr/bin/env python3
import json
import os
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font

# Constants
OUTPUT_PATH = Path("Test_Output/GPIO/TestPlan.xlsx")
SHEET_NAME = "TestPlan"
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


def to_text(value):
    if value is None:
        return ""
    if isinstance(value, (list, tuple)):
        return "\n".join(str(v) for v in value)
    return str(value)


def format_steps(steps):
    if steps is None:
        return ""
    if isinstance(steps, (list, tuple)):
        # Preserve order; number steps
        return "\n".join(f"{i+1}. {to_text(s)}" for i, s in enumerate(steps))
    return to_text(steps)


def main():
    # Read consolidated JSON from repository root
    input_file = Path("testplan_input.json")
    if not input_file.exists():
        raise SystemExit("Missing testplan_input.json at repo root. The workflow should have created it.")

    with input_file.open("r", encoding="utf-8") as f:
        data = json.load(f)

    # Validate structure
    testcases = data.get("testcases", [])
    if not isinstance(testcases, list):
        raise SystemExit("Invalid JSON: 'testcases' must be a list")

    # Prepare workbook
    wb = Workbook()
    # Remove the default sheet to avoid duplicates
    default_title = wb.active.title
    ws = wb.active
    ws.title = SHEET_NAME

    # Write headers with bold font
    bold = Font(bold=True)
    for col_idx, header in enumerate(HEADERS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=header)
        cell.font = bold

    # Freeze header row
    ws.freeze_panes = "A2"

    # Fill rows
    ip_name_global = data.get("ip_name", "")
    for i, t in enumerate(testcases, start=1):
        row = [
            i,  # Index
            t.get("ip_name") or ip_name_global or "",  # SS / Module
            to_text(t.get("feature", "")),  # Feature
            to_text(t.get("name") or t.get("id") or ""),  # Test Case Name
            to_text(t.get("description", "")),  # Test Description
            to_text(t.get("speed", "")),  # Speed
            to_text(t.get("mode", "")),  # Mode
            to_text(t.get("remarks", "")),  # Remarks
            format_steps(t.get("steps")),  # Test Steps / Procedure
            to_text(t.get("impacted_registers", "")),  # Impacted Registers
            to_text(t.get("expected_result", "")),  # Validation / Acceptance Criteria
            to_text(t.get("gap_analysis", "")),  # Gap Analysis
        ]
        for col_idx, value in enumerate(row, start=1):
            ws.cell(row=i + 1, column=col_idx, value=value)

    # Ensure directory exists and save a REAL .xlsx workbook
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    wb.save(str(OUTPUT_PATH))
    print(f"Saved Excel to {OUTPUT_PATH}")


if __name__ == "__main__":
    main()
