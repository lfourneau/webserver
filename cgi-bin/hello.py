#!/usr/bin/env python3
"""Greets the user. Works with GET (query string) and POST (form body)."""
import common as c

fields, body = c.params()
name = fields.get("name", "").strip() or "stranger"

rows = "".join(
    f"<tr><td class=\"mono\">{c.esc(k)}</td><td>{c.esc(v)}</td></tr>"
    for k, v in fields.items()
) or '<tr><td colspan="2" class="muted">No fields received.</td></tr>'

content = f"""
<section class="hero">
    <h1>Hello, {c.esc(name)}!</h1>
    <p>This page was written by a Python script, not read from a file.</p>
</section>

<div class="grid-2">
    <div class="card">
        <h2>What the script received</h2>
        <table>
            <tbody>
                <tr><td>Method</td><td><span class="badge {c.method().lower()}">{c.esc(c.method())}</span></td></tr>
                <tr><td>Query string</td><td class="mono">{c.esc(c.query_string()) or '<span class="muted">(empty)</span>'}</td></tr>
                <tr><td>Body</td><td class="mono">{c.esc(body) or '<span class="muted">(empty)</span>'}</td></tr>
            </tbody>
        </table>
    </div>
    <div class="card">
        <h2>Parsed fields</h2>
        <table>
            <thead><tr><th>Name</th><th>Value</th></tr></thead>
            <tbody>{rows}</tbody>
        </table>
    </div>
</div>
{c.generated_by("cgi-bin/hello.py")}
<p><a class="btn secondary" href="/get.html">GET form</a> <a class="btn secondary" href="/post.html">POST form</a></p>
"""

print(c.page(f"Hello {name}", content))
