#!/usr/bin/env python3
"""Runs for 10 seconds: a server with a CGI timeout should answer 504.

Without a timeout the server waits, then sends this page.
"""
import time
import common as c

time.sleep(10)

print(c.page("Slow script", """
<h1 class="page-title">Finished after 10 seconds</h1>
<p class="page-intro">The server waited for the whole script. With a CGI timeout shorter
   than 10 seconds, you would have got a 504 Gateway Timeout instead.</p>
"""))
