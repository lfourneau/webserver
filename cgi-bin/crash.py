#!/usr/bin/env python3
"""Crashes on purpose: the server should answer 500 (or 502)."""
import sys

sys.stderr.write("crash.py: failing on purpose to test error handling\n")
raise RuntimeError("This script crashes on purpose.")
