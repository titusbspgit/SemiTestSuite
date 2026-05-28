#!/usr/bin/env python3
import json
import sys
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

# Column headers for the required sheet structure
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
    "Imparted Registers" if False else "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Gap Analysis",
]

MAPPED_KEYS = {
    "component",
    "title",
    "description",
    "speed",
    "mode",
    "steps",
    "expected_result",
    "impacted_registers",
    "gap_analysis",
}


def load_json(path: str):
    if not os.path.exists(path):
        raise FileNotFoundError(f"JSON file not found: {path}")
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if isinstance(data, dict):
        data = [data]
    if not isinstance(data, list):
        raise ValueError("JSON must be an array of objects or a single object")
    return data


def to_row(idx: int, obj: dict):
    # Feature: prefer explicit 'feature', else tags joined, else empty
    feature = obj.get("feature")
    if not feature:
        tags = obj.get("tags")
        if isinstance(tags, list):
            feature = ", ".join(map(str, tags))
        else:
            feature = ""

    # Remarks: include unmapped keys as a compact JSON string to avoid data loss
    extras = {k: v for k, v in obj.items() if k not in MAPPED_KEYS and k not in ("feature", "tags")}
    remarks = json.dumps(extras, ensure_ascii=False, separators=(",", ":")) if extras else ""

    return [
        idx,
        obj.get("component", ""),
        feature,
        obj.get("title", ""),
        obj.get("description", ""),
        obj.get("speed", ""),
        obj.get("mode", ""),
        remarks,
        obj.get("steps", ""),
        obj.get("impacted_registers", ""),
        obj.get("expected_result", ""),
        obj.get("gap_analysis", ""),
    ]


def save_excel(rows, out_dir: str):
    os.makedirs(out_dir, exist_ok=True)
    # IST timestamp
    ts = datetime.now(ZoneInfo("Asia/Kolkata")).strftime("%Y%m%d_%H%M%S")
    out_path = os.path.join(out_dir, f"testplan_{ts}.xlsx")

    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Write header
    ws.append(HEADERS)
    # Bold headers
    for cell in ws[1]:
        cell.font = Font(bold=True)
    # Freeze first row
    ws.freeze_panes = "A2"

    # Write rows
    for r in rows:
        ws.append(r)

    # Optional: autosize columns
    for col in ws.columns:
        max_length = 0
        column = col[0].column_letter
        for cell in col:
            try:
                val = str(cell.value) if cell.value is not None else ""
                if len(val) > max_length:
                    max_length = len(val)
            except Exception:
                pass
        ws.column_dimensions[column].width = min(max(12, max_length + 2), 80)

    wb.save(out_path)
    print(out_path)
    return out_path


def main():
    in_path = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("INPUT_JSON", "Test_Output/PCIE/testplan.json")
    out_dir = sys.argv[2] if len(sys.argv) > 2 else os.environ.get("OUTPUT_DIR", os.path.dirname(in_path) or ".")

    data = load_json(in_path)
    # Validate array of dicts
    rows = []
    for i, item in enumerate(data, start=1):
        if not isinstance(item, dict):
            raise ValueError(f"Item {i} is not an object")
        rows.append(to_row(i, item))

    save_excel(rows, out_dir)


if __name__ == "__main__":
    main()
