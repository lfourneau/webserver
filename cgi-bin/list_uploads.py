#!/usr/bin/env python3
"""Prints the files in upload/ as JSON (used by the DELETE page)."""
import datetime
import json
import os

UPLOAD_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "upload")

files = []
try:
    for name in sorted(os.listdir(UPLOAD_DIR)):
        path = os.path.join(UPLOAD_DIR, name)
        if not os.path.isfile(path) or name.startswith("."):
            continue
        st = os.stat(path)
        files.append({
            "name": name,
            "size": st.st_size,
            "modified": datetime.datetime.fromtimestamp(st.st_mtime).strftime("%Y-%m-%d %H:%M:%S"),
        })
except OSError as e:
    print(json.dumps({"files": [], "error": str(e)}))
else:
    print(json.dumps({"files": files}))
