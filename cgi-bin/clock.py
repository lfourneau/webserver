#!/usr/bin/env python3
"""Shows the server's current date and time."""
import datetime
import common as c

now = datetime.datetime.now()

content = f"""
<section class="hero">
    <h1 style="font-size: 3rem; font-family: var(--mono);">{now:%H:%M:%S}</h1>
    <p>{now:%A %d %B %Y}</p>
</section>
<div class="card">
    <h2>Why this matters</h2>
    <p class="muted">A static file is the same every time. This page is built when you ask for it,
       so reloading it shows a new time.</p>
    <a class="btn" href="/cgi-bin/clock.py">Reload</a>
</div>
{c.generated_by("cgi-bin/clock.py")}
"""

print(c.page("Server clock", content))
