#!/usr/bin/env python3
"""Shows every environment variable the server passed to the script."""
import os
import common as c

# Variables a CGI script usually receives; highlighted when present.
CGI_VARS = {
    "REQUEST_METHOD", "QUERY_STRING", "CONTENT_LENGTH", "CONTENT_TYPE",
    "SCRIPT_NAME", "SCRIPT_FILENAME", "PATH_INFO", "SERVER_NAME", "SERVER_PORT",
    "SERVER_PROTOCOL", "SERVER_SOFTWARE", "GATEWAY_INTERFACE", "REMOTE_ADDR",
    "REDIRECT_STATUS",
}


def is_cgi(name):
    return name in CGI_VARS or name.startswith("HTTP_")


names = sorted(os.environ, key=lambda n: (not is_cgi(n), n))
rows = "".join(
    f'<tr><td class="mono">{"<strong>" if is_cgi(n) else ""}{c.esc(n)}'
    f'{"</strong>" if is_cgi(n) else ""}</td>'
    f'<td class="mono">{c.esc(os.environ[n])}</td></tr>'
    for n in names
)

content = f"""
<h1 class="page-title">CGI environment</h1>
<p class="page-intro">The server gives the request to the script through these variables.
   CGI variables are in bold and listed first. Try adding <code>?foo=bar</code> to the URL.</p>
<div class="card">
    <div class="table-wrap">
        <table>
            <thead><tr><th>Variable</th><th>Value</th></tr></thead>
            <tbody>{rows}</tbody>
        </table>
    </div>
</div>
{c.generated_by("cgi-bin/env.py")}
"""

print(c.page("CGI environment", content))
