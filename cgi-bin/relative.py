#!/usr/bin/env python3
"""Opens a file with a relative path: works only if the server runs the
script in its own directory (cgi-bin/), as the subject requires."""
import os
import common as c

try:
    with open("../upload/welcome.txt") as f:
        content = f.read()
    result = f'<p>Read <code>../upload/welcome.txt</code>:</p><pre>{c.esc(content)}</pre>'
except OSError as e:
    result = f'<p>Could not open <code>../upload/welcome.txt</code>: {c.esc(e)}</p>'

print(c.page("Relative path", f"""
<h1 class="page-title">CGI working directory</h1>
<p>Current directory: <code>{c.esc(os.getcwd())}</code></p>
{result}
"""))
