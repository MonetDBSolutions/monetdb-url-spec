#!/usr/bin/env python3

from collections import defaultdict
import hashlib
import re

url_locations = defaultdict(lambda:[])
location_hashes = dict()

pattern = re.compile(r'^(ACCEPT|REJECT|PARSE)\s+(.*?)\s*$', re.IGNORECASE)

block = None
block_locations = []
for i, line in enumerate(open('x.md').readlines(), 1):
    if block is None and line.strip() == '```test':
        block = ""
        block_locations = []
        continue
    if block is not None and line.strip() == '```':
        h = hashlib.sha256(bytes(block, 'utf-8')).hexdigest()[:6]
        for loc in block_locations:
            location_hashes[loc] = h
        block = None
    if block is not None:
        block += line

    m = pattern.match(line)
    if m:
        url = m.group(2)
        block_locations.append(i)
        url_locations[url].append(i)

urls = list(url_locations.keys())
urls.sort(key=lambda u: url_locations[u])

for url in urls:
    locs = url_locations[url]
    if len(locs) < 2:
        continue
    print(url)
    for loc in locs:
        h = location_hashes[loc]
        print(f"    {loc}:{h}")
