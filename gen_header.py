#!/usr/bin/env python

import sys
import re

filepath = sys.argv[1]

if not filepath.endswith(".c"):
    print("that is not a c source file.")
    exit(1)

try:
    filename = re.search("^.*?(\w+)\.c$", filepath)[1]
    headerdef = filename.upper().replace(".", "_") + "_H"
except Exception as e:
    print("couldn't parse filename:\n")
    print(e)
    exit(1)

with open(filepath[:-1] + "h", 'w') as f:
    f.write(f"#ifndef {headerdef}\n#define {headerdef}\n\n\n\n#endif")
