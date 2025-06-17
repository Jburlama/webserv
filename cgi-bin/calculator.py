#!/usr/bin/env python3

import os
import sys
import html
import urllib.parse

def main():
    # Get request method
    method = os.environ.get('PATH_INFO', 'GET')
    result = ""
    
    # Get data from PATH_INFO
    path_info = os.environ.get('PATH_INFO', '')
    
    if path_info:
        # Parse path_info
        a, b, op = "", "", "+"
        params = urllib.parse.parse_qs(path_info)
        
        if 'a' in params:
            a = params['a'][0]
        if 'b' in params:
            b = params['b'][0]
        if 'op' in params:
            op = params['op'][0]
        
        # Perform calculation
        try:
            num1 = float(a) if a else 0
            num2 = float(b) if b else 0
            
            if op == '+':
                result = f"{num1} + {num2} = {num1 + num2}"
            elif op == '-':
                result = f"{num1} - {num2} = {num1 - num2}"
            elif op == '*':
                result = f"{num1} × {num2} = {num1 * num2}"
            elif op == '/':
                if num2 == 0:
                    result = "Error: Division by zero"
                else:
                    result = f"{num1} ÷ {num2} = {num1 / num2}"
            else:
                result = "Error: Invalid operation"
        except ValueError:
            result = "Error: Invalid input numbers"
    
    # Output HTML response
    print("""<!DOCTYPE html>
<html>
<head>
    <title>Calculator</title>
    <style>
        body {{ 
            font-family: Arial, sans-serif; 
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
            background-color: #f5f5f5;
        }}
        .calculator {{
            background: white;
            padding: 2rem;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            width: 300px;
        }}
        h1 {{
            text-align: center;
            color: #333;
            margin-top: 0;
        }}
        .result-area {{
            min-height: 60px;
            padding: 1rem;
            margin-bottom: 1rem;
            background: #f8f9fa;
            border: 1px solid #e9ecef;
            border-radius: 4px;
            text-align: center;
            font-size: 1.2rem;
            color: #333;
        }}
        .form-group {{
            margin-bottom: 1rem;
        }}
        label {{
            display: block;
            margin-bottom: 0.5rem;
            color: #495057;
            font-weight: bold;
        }}
        input, select {{
            width: 100%;
            padding: 0.75rem;
            border: 1px solid #ced4da;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 1rem;
        }}
        input[type="submit"] {{
            background: #007bff;
            color: white;
            border: none;
            padding: 0.75rem;
            cursor: pointer;
            font-size: 1rem;
            border-radius: 4px;
            transition: background 0.3s;
            font-weight: bold;
        }}
        input[type="submit"]:hover {{
            background: #0069d9;
        }}
        .error {{
            color: #dc3545;
            font-weight: bold;
        }}
    </style>
</head>
<body>
    <div class="calculator">
        <h1>Simple Calculator</h1>
        
        <!-- Result area (empty for GET, filled for POST) -->
        <div class="result-area">""")
    
    # Output result if available
    if result:
        if "Error" in result:
            print(f'<div class="error">{result}</div>')
        else:
            print(result)
    
    print("""        </div>
        
        <form method="GET">
            <div class="form-group">
                <label for="a">First number:</label>
                <input type="number" step="any" name="a" id="a" required>
            </div>
            
            <div class="form-group">
                <label for="b">Second number:</label>
                <input type="number" step="any" name="b" id="b" required>
            </div>
            
            <div class="form-group">
                <label for="op">Operation:</label>
                <select name="op" id="op">
                    <option value="+">Addition (+)</option>
                    <option value="-">Subtraction (-)</option>
                    <option value="*">Multiplication (×)</option>
                    <option value="/">Division (÷)</option>
                </select>
            </div>
            
            <input type="submit" value="Calculate">
        </form>
    </div>
</body>
</html>""")

if __name__ == "__main__":
    main()
