#!/usr/bin/env python3

import re

content = open('monetdb-url.md').read()

keywords = set(m.group(1) for m in re.finditer(r'\*\*([a-z_]+)\*\*', content))
for w in sorted(keywords):
    print(w)