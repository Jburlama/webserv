#!/usr/bin/env python3

import os
import cgi
import html
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

# Main CGI processing
form = cgi.FieldStorage()
action = form.getvalue('action')
message = ""

# Process actions
if action == 'upload':
    file_item = form['file']
    if file_item.filename:
        message = save_uploaded_file(file_item)

# Output HTML response
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
    <script>
        async function deleteFile(filename) {{
            if (!confirm('Are you sure you want to delete ' + filename + '?')) {{
                return;
            }}
            
            try {{
                const response = await fetch('/upload/' + encodeURIComponent(filename), {{
                    method: 'DELETE'
                }});
                
                if (response.ok) {{
                    const result = await response.text();
                    alert(result);
                    location.reload(); // Refresh page after deletion
                }} else {{
                    const error = await response.text();
                    throw new Error(error || 'Deletion failed');
                }}
            }} catch (error) {{
                alert('Error: ' + error.message);
            }}
        }}
    </script>
</head>
<body>
    <h1>File Manager</h1>
    
    {f'<div class="message">{html.escape(message)}</div>' if message else ''}
    
    
    <h2>Files in {UPLOAD_DIR}/</h2>
    <table>
        <tr>
            <th>Filename</th>
        </tr>
""")

# List files with actions
for filename in list_files():
    safe_filename = html.escape(filename)
    print(f"""
        <tr>
            <td>{safe_filename}</td>
        </tr>
    """)

print("""
    </table>
</body>
</html>
""")
