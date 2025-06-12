#!/usr/bin/env python3

import os
import cgi
import shutil
import sys

UPLOAD_DIR = "upload"
MAX_UPLOAD_SIZE = 1024 * 1024  # 1MB

# Ensure upload directory exists
os.makedirs(UPLOAD_DIR, exist_ok=True)

def list_files():
    files = sorted(os.listdir(UPLOAD_DIR))
    return [
        f for f in files
        if os.path.isfile(os.path.join(UPLOAD_DIR, f))
    ]

def save_uploaded_file(file_item):
    filename = os.path.basename(file_item.filename)
    filepath = os.path.join(UPLOAD_DIR, filename)
    
    # Prevent path traversal
    if not os.path.abspath(filepath).startswith(os.path.abspath(UPLOAD_DIR)):
        return "Invalid filename"
    
    with open(filepath, 'wb') as f:
        while True:
            chunk = file_item.file.read(8192)
            if not chunk:
                break
            f.write(chunk)
    return f"Uploaded: {filename}"

def delete_file(filename):
    filepath = os.path.join(UPLOAD_DIR, filename)
    
    # Prevent path traversal
    if not os.path.abspath(filepath).startswith(os.path.abspath(UPLOAD_DIR)):
        return "Invalid filename"
    
    if os.path.exists(filepath):
        os.remove(filepath)
        return f"Deleted: {filename}"
    return "File not found"

# Main CGI processing
form = cgi.FieldStorage()
action = form.getvalue('action')
message = ""

# Process actions
if action == 'upload':
    file_item = form['file']
    if file_item.filename:
        message = save_uploaded_file(file_item)
elif action == 'delete':
    filename = form.getvalue('filename')
    if filename:
        message = delete_file(filename)

# Output HTML response (body only)
print(f"""
<!DOCTYPE html>
<html>
<head>
    <title>File Manager</title>
    <style>
        body {{ font-family: sans-serif; max-width: 800px; margin: 0 auto; }}
        table {{ width: 100%; border-collapse: collapse; }}
        th, td {{ padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }}
        .actions {{ display: flex; gap: 10px; }}
        .message {{ padding: 10px; background: #f0f0f0; margin: 20px 0; }}
    </style>
</head>
<body>
    <h1>File Manager</h1>
    
    {f'<div class="message">{message}</div>' if message else ''}
    
    <h2>Upload File</h2>
    <form method="post" enctype="multipart/form-data">
        <input type="file" name="file" required>
        <input type="submit" name="action" value="upload">
    </form>
    
    <h2>Files in {UPLOAD_DIR}/</h2>
    <table>
        <tr>
            <th>Filename</th>
            <th>Actions</th>
        </tr>
""")

# List files with actions
for filename in list_files():
    print(f"""
        <tr>
            <td>{filename}</td>
            <td class="actions">
                <a href="{os.path.join(UPLOAD_DIR, filename)}" download>
                    <button>Download</button>
                </a>
                <form method="post" style="display: inline">
                    <input type="hidden" name="filename" value="{filename}">
                    <input type="submit" name="action" value="delete">
                </form>
            </td>
        </tr>
    """)

print("""
    </table>
</body>
</html>
""")
