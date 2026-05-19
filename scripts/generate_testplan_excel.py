#!/usr/bin/env python3
import json
import os
import argparse
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment
from openpyxl.utils import get_column_letter

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
    "Gap Analysis"
]

def to_ist_timestamp():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime('%Y%m%d_%H%M%S')

def normalize_rows(data):
    rows = []
    for item in data:
        row = {}
        for h in HEADERS:
            row[h] = item.get(h, "")
        rows.append(row)
    return rows

def main():
    ap = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON array")
    ap.add_argument('--json', dest='json_path', default=os.getenv('JSON_PATH','Test_Output/GPIO/testplan_data.json'))
    ap.add_argument('--outdir', dest='outdir', default=os.getenv('OUTPUT_DIR','Test_Output/GPIO'))
    ap.add_argument('--sheet', dest='sheet', default=os.getenv('SHEET_NAME','TestPlan'))
    args = ap.parse_args()

    # Load JSON
    with open(args.json_path, 'r', encoding='utf-8') as f:
        content = json.load(f)

    # Accept array or object with 'data'/'testcases'
    if isinstance(content, dict):
        if 'data' in content and isinstance(content['data'], list):
            data = content['data']
        elif 'testcases' in content and isinstance(content['testcases'], list):
            data = []
            for i, tc in enumerate(content['testcases'], start=1):
                steps = tc.get('steps', [])
                if isinstance(steps, list):
                    steps_str = "\n".join([f"{idx+1}) {s}" for idx, s in enumerate(steps)])
                else:
                    steps_str = str(steps)
                data.append({
                    "Index": str(i),
                    "SS / Module": tc.get("module",""),
                    "Feature": tc.get("feature",""),
                    "Test Case Name": tc.get("name",""),
                    "Test Description": tc.get("description",""),
                    "Speed": tc.get("speed",""),
                    "Mode": tc.get("mode",""),
                    "Remarks": tc.get("remarks",""),
                    "Test Steps / Procedure": steps_str,
                    "Impacted Registers": tc.get("registers",""),
                    "Validation / Acceptance Criteria": tc.get("expected_result",""),
                    "Gap Analysis": tc.get("gap",""),
                })
        else:
            raise ValueError("Unsupported JSON object format: expected 'data' or 'testcases' array")
    elif isinstance(content, list):
        data = content
    else:
        raise ValueError("json_data must be an array or an object with 'data' or 'testcases'")

    if not isinstance(data, list):
        raise ValueError("json_data is not an array")

    rows = normalize_rows(data)

    # Create workbook and sheet
    wb = Workbook()
    ws = wb.active
    ws.title = args.sheet

    # Header row with bold font
    header_font = Font(bold=True)
    wrap = Alignment(wrap_text=True, vertical="top")
    for col_idx, h in enumerate(HEADERS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=h)
        cell.font = header_font

    # Data rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, h in enumerate(HEADERS, start=1):
            value = row.get(h, "")
            if h == "Test Steps / Procedure" and isinstance(value, list):
                value = "\n".join([f"{i+1}) {step}" for i, step in enumerate(value)])
            ws.cell(row=r_idx, column=c_idx, value=value)

    # Freeze first row
    ws.freeze_panes = 'A2'

    # Apply wrapping and set reasonable column widths
    for c_idx, h in enumerate(HEADERS, start=1):
        col_letter = get_column_letter(c_idx)
        for cell in ws[col_letter]:
            cell.alignment = wrap
        if h in ("Index","Speed","Mode"):
            ws.column_dimensions[col_letter].width = 10
        elif h in ("SS / Module","Feature"):
            ws.column_dimensions[col_letter].width = 24
        else:
            ws.column_dimensions[col_letter].width = 40

    # Ensure output dir exists
    os.makedirs(args.outdir, exist_ok=True)

    # Compute IST timestamp and save
    ts = to_ist_timestamp()
    out_path = os.path.join(args.outdir, f"testplan_{ts}.xlsx")
    wb.save(out_path)
    print(out_path)

if __name__ == '__main__':
    main()
