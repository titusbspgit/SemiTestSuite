#!/usr/bin/env python3

import json
import os
import sys
from datetime import datetime
from zoneinfo import ZoneInfo
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


def main() -> int:
    input_json_path = os.environ.get("INPUT_JSON_PATH", "TestRepo/gpio/test_plan.json")
    output_dir = os.environ.get("OUTPUT_DIR", "Test_Output/GPIO")

    # Step 1: Validate JSON
    with open(input_json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    if not isinstance(data, list):
        print("ERROR: json_data must be a JSON array.", file=sys.stderr)
        return 1

    for i, row in enumerate(data):
        if not isinstance(row, dict):
            print(f"ERROR: Element at index {i} is not an object.", file=sys.stderr)
            return 1

    # Step 2: Create Excel workbook and sheet
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Header row with bold formatting
    ws.append(COLUMNS)
    for cell in ws[1]:
        cell.font = Font(bold=True)

    # Freeze first row
    ws.freeze_panes = "A2"

    # Preserve data verbatim per required column mapping
    for row in data:
        ws.append([
            "" if row.get(col) is None else str(row.get(col))
            for col in COLUMNS
        ])

    # Step 3: Save as real .xlsx with IST timestamp
    os.makedirs(output_dir, exist_ok=True)
    ist = ZoneInfo("Asia/Kolkata")
    timestamp = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    output_path = os.path.join(output_dir, f"testplan_{timestamp}.xlsx")
    wb.save(output_path)

    print(f"Generated Excel: {output_path}")

    # Expose path to subsequent workflow steps if running in GitHub Actions
    github_output = os.environ.get("GITHUB_OUTPUT")
    if github_output:
        with open(github_output, "a", encoding="utf-8") as fh:
            fh.write(f"excel_path={output_path}\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
