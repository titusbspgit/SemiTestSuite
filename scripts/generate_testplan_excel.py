import json
import os
from datetime import datetime, timezone, timedelta
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


def main():
    # Step 1: Validate and load JSON data
    input_path = os.path.join("data", "testplan.json")
    with open(input_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    if not isinstance(data, list):
        raise SystemExit("Input JSON is not an array")

    rows = []
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            raise SystemExit(f"Item at index {i} is not an object")
        # Preserve exact data values; blank for missing keys
        row = [item.get(col, "") for col in COLUMNS]
        rows.append(row)

    # Step 2: Create Excel workbook and sheet with formatting
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Header
    ws.append(COLUMNS)
    for cell in ws[1]:
        cell.font = Font(bold=True)

    # Freeze first row
    ws.freeze_panes = "A2"

    # Data rows
    for row in rows:
        ws.append(row)

    # Step 3: Save with IST timestamp
    ist = timezone(timedelta(hours=5, minutes=30))
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")

    output_dir = os.path.join("Test_Output", "GPIO")
    os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"testplan_{ts}.xlsx")
    wb.save(output_path)

    print(output_path)


if __name__ == "__main__":
    main()
