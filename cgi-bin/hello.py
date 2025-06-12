#!/usr/bin/env python3
import os
import sys

print("<html lang='en'>")
print("<head>")
print("<meta charset='UTF-8'>")
print("<meta name='viewport' content='width=device-width, initial-scale=1'>")
print("<title> Webserv </title>")
print("""
        <style>
            body {
                background: #1f1f1f;
                color: #ffffff;
                text-align: center;
                height: 40vh;
            }
        </style>
        """
)
print("</head>")

print("<body>")
print("<h1>Hello</h1>")
print("</body>")

print("</html>")

# Print all environment variables
# for var_name, value in os.environ.items():
#     print(f"{var_name}={value}")
