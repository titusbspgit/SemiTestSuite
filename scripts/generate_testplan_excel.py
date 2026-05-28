#!/usr/bin/env python3
import sys, json, os
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
    tz = ZoneInfo("Asia/Kolkata")
except Exception:
    try:
        import pytz
        tz = pytz.timezone("Asia/Kolkata")
    except Exception:
        tz = None
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment

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

def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if isinstance(data, list):
        rows = data
    elif isinstance(data, dict) and isinstance(data.get('data'), list):
        rows = data['data']
    else:
        raise ValueError("Invalid json_data: must be array or object with 'data' array")
    return rows

def autosize(ws):
    for col_idx, col_name in enumerate(COLUMNS, start=1):
        max_len = len(col_name)
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            cell = row[0]
            if cell.value is None:
                continue
            s = str(cell.value)
            max_line = max((len(line) for line in s.splitlines()), default=0)
            if max_line > max_len:
                max_len = max_line
        width = min(max_len + 2, 100)
        ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = width

def build_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"
    ws.append(COLUMNS)
    for cell in ws[1]:
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"
    wrap_cols = {
        COLUMNS.index("Test Steps / Procedure") + 1,
        COLUMNS.index("Test Description") + 1,
        COLUMNS.index("Remarks") + 1,
        COLUMNS.index("Validation / Acceptance Criteria") + 1,
        COLUMNS.index("Gap Analysis") + 1,
    }
    for r in rows:
        row = [r.get(k, "") for k in COLUMNS]
        ws.append(row)
        last_row = ws.max_row
        for cidx in wrap_cols:
            cell = ws.cell(row=last_row, column=cidx)
            cell.alignment = Alignment(wrap_text=True, vertical="top")
    for row in ws.iter_rows(min_row=2):
        for cell in row:
            if cell.alignment is None or cell.alignment.vertical is None:
                cell.alignment = Alignment(vertical="top", wrap_text=bool("\n" in str(cell.value) if cell.value else False))
    autosize(ws)
    return wb

def main():
    if len(sys.argv) < 1:
        print("Usage: generate_testplan_excel.py <json_path>", file=sys.stderr)
        sys.exit(2)
    json_path = sys.argv[1] if len(sys.argv) > 1 else 'data/testplans/PCIE_testplan.json'
    rows = load_json(json_path)
    wb = build_workbook(rows)
    now = datetime.now(tz) if tz else datetime.utcnow()
    ts = now.strftime("%Y%m%d_%H%M%S")
    out_dir = os.path.join("Test_Output", "PCIE")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, f"testplan_{ts}.xlsx")
    wb.save(out_path)
    print(out_path)

if __name__ == "__main__":
    main()
