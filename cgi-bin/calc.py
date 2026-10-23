#!/usr/bin/env python3
"""Small calculator: a, b, op (add|sub|mul|div), from GET or POST.

With format=text it prints a single line instead of a full page.
"""
import common as c

SYMBOLS = {"add": "+", "sub": "-", "mul": "×", "div": "÷"}

fields, _ = c.params()
op = fields.get("op", "add")

try:
    a = float(fields.get("a", "0"))
    b = float(fields.get("b", "0"))
    if op == "add":
        result = a + b
    elif op == "sub":
        result = a - b
    elif op == "mul":
        result = a * b
    elif op == "div":
        result = a / b
    else:
        raise ValueError(f"unknown operation '{op}'")
    answer = f"{a:g} {SYMBOLS[op]} {b:g} = {result:g}"
except ZeroDivisionError:
    answer = "Error: division by zero"
except ValueError as e:
    answer = f"Error: {e}"

if fields.get("format") == "text":
    print(f"[python] {answer}  (method: {c.method()})")
else:
    content = f"""
<section class="hero"><h1 style="font-family: var(--mono);">{c.esc(answer)}</h1>
<p>Computed by Python.</p></section>
{c.generated_by("cgi-bin/calc.py")}
<p><a class="btn secondary" href="/cgi.html">Back to CGI page</a></p>
"""
    print(c.page("Calculator", content))
