#!/usr/bin/env python3
import json
import argparse
import os
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font

def serialize(value):
    if isinstance(value, (dict, list)):
        return json.dumps(value, ensure_ascii=False, separators=(",", ":"))
    return value if value is not None else ""

COLUMNS = [
    "id",
    "name",
    "description",
    "folder",
    "file",
    "steps",
    "parameters",
    "prerequisites",
    "expected_results",
    "tags",
    "priority",
    "type",
    "requirements",
    "github_file_url",
    "raw_file_url",
]


def build_row(test: dict):
    links = test.get("links", {}) or {}
    return [
        serialize(test.get("id", "")),
        serialize(test.get("name", "")),
        serialize(test.get("description", "")),
        serialize(test.get("folder", "")),
        serialize(test.get("file", "")),
        serialize(test.get("steps", [])),
        serialize(test.get("parameters", {})),
        serialize(test.get("prerequisites", [])),
        serialize(test.get("expected_results", [])),
        serialize(test.get("tags", [])),
        serialize(test.get("priority", "")),
        serialize(test.get("type", "")),
        serialize(test.get("requirements", [])),
        serialize(links.get("github_file_url", "")),
        serialize(links.get("raw_file_url", "")),
    ]


def generate_excel(input_json_path: Path, out_path: Path, sheet_name: str = "GPIO-TestPlan"):
    with open(input_json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    # Validation
    if not isinstance(data, dict) or "tests" not in data or not isinstance(data["tests"], list):
        raise SystemExit("Invalid json_data: expected object with non-empty 'tests' array")

    tests = data["tests"]

    wb = Workbook()
    ws = wb.active
    ws.title = sheet_name

    # Header
    header_font = Font(bold=True)
    ws.append(COLUMNS)
    for cell in ws[1]:
        cell.font = header_font

    # Rows
    for t in tests:
        ws.append(build_row(t))

    # Freeze header row
    ws.freeze_panes = "A2"

    # Ensure directory exists
    out_path.parent.mkdir(parents=True, exist_ok=True)

    wb.save(out_path)


def main():
    parser = argparse.ArgumentParser(description="Generate GPIO TestPlan Excel from JSON")
    parser.add_argument("--input", required=True, help="Path to input JSON file")
    parser.add_argument("--out", required=True, help="Output .xlsx path (e.g., Test_Output/GPIO/TestPlan.xlsx)")
    parser.add_argument("--sheet", default="GPIO-TestPlan", help="Worksheet name")
    args = parser.parse_args()

    input_json = Path(args.input)
    out_path = Path(args.out)

    generate_excel(input_json, out_path, args.sheet)

if __name__ == "__main__":
    main()
