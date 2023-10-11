#!/usr/bin/env python3


import argparse
import difflib
import os
import re
from typing import List

from monetparameters import PARAMS

SPEC = os.path.join(os.path.dirname(__file__), 'monetdb-url.md')


ALSO_ALLOW = [
    'connect_unix', 'connect_tcp', 'connect_port', 'connect_scan',
    'connect_tls_verify', 'connect_certhash_digits',
    'connect_binary',
    'param1', 'param2', 'value1', 'value2',
]


class Line(str):
    """A Line is a string that remembers which file and line number it cames from"""
    file: str
    idx: int
    nr: int

    def __new__(cls, text: str, file: str, idx: int):
        line = super().__new__(cls, text)
        line.file = file
        line.idx = idx
        line.nr = idx + 1
        return line

    @property
    def location(self):
        return f"{self.file}:{self.nr}"


def read_spec(spec=SPEC, relative_to=os.curdir) -> List[Line]:
    """Return all lines of the spec"""
    f = os.path.relpath(spec, relative_to)
    return [Line(line.rstrip(), f, i) for i, line in enumerate(open(spec).readlines())]


def split_section(section: str, lines: List[Line]) -> (List[Line], List[Line], List[Line]):
    # find the section
    start = None
    end = None
    header = None
    for line in lines:
        if start is None:
            if line.startswith('#') and section in line:
                start = line
                header = ''
                for c in line:
                    if c == '#':
                        header += c
                    else:
                        break
        elif end is None:
            if line.startswith('#') and not line.startswith(header + '#'):
                end = line
        else:
            if line.startswith('#') and section in line:
                exit(f"Section {section!r} matches both {start} and {line.nr}")

    if start is None:
        exit(f"Section {section!r} not found")
    head = lines[:start.idx]
    sec = lines[start.idx:end.idx]
    tail = lines[end.idx:]
    return (head, sec, tail)


def dump_table() -> List[str]:
    # tabulate
    head = ["Parameter", "Type", "Default", "Remark"]
    rows = [head, ['-' for f in head]]
    for p in PARAMS:
        core = '(core) ' if p.core else ''
        row = [
            f"**{p.name}**",
            p.typ,
            p.default or "--",
            core + p.descr
        ]
        rows.append(row)

    # tabulate
    widths = [0, 0, 0, 0]
    for row in rows:
        widths = [max(w, len(f)) for w, f in zip(widths, row, strict=True)]
    lines = []
    for row in rows:
        if row[0] == '-':
            row = ['-' * n for n in widths]
        line = ''
        for w, f in zip(widths, row, strict=True):
            line += '| ' + f.ljust(w) + ' '
        line += '|'
        lines.append(line)

    return lines


def check_params_table(lines: List[Line]):
    ok = True
    section_name = 'Parameters'
    head, sec, tail = split_section(section_name, lines)
    current = sec[:]
    while current and not current[0].startswith('|'):
        current = current[1:]
    while current and not current[-1].startswith('|'):
        current = current[:-1]

    correct = dump_table()
    if current != correct:
        print(f"Params table differences near line {len(head) + 1}: ")

        diff = difflib.unified_diff(
            current, correct, fromfile='CURRENT', tofile='CORRECT')
        for line in diff:
            print(line.rstrip())
        print
        ok = False

    pattern = r'^The following (\d+)'
    pat = re.compile(pattern)
    np = len(PARAMS)
    for line in sec:
        m = pat.match(line)
        if m:
            n = int(m.group(1))
            if n != np:
                ok = False
                print(f"{line.location}: expected {np} parameters, not {n}")
            break
    else:
        ok = False
        print(f"Section  '{section_name}' pattern '{pattern}' not found")

    return ok


PARAM_PATTERN = re.compile(r'[*][*](\w+)[*][*]|<b>(\w+)</b>')


def check_bad_underscores(lines: List[Line]):
    ok = True
    valid = set(p.name for p in PARAMS) | set(ALSO_ALLOW)
    for line in lines:
        # if 'OLD CRUFT' in line:
        #     break
        for m in PARAM_PATTERN.finditer(line):
            word = m.group(1) or m.group(2)
            if word in valid:
                continue
            ok = False
            if word.startswith('connect_'):
                print(f"{line.location}: unknown setting {word!r}, maybe add it to ALSO_ALLOW?")
            else:
                print(f"{line.location}: unknown setting {word!r}")
    return ok


def check_whitespace(lines: List[Line]):
    ok = True
    for line in lines:
        if line.startswith('#') and line.idx >= 2:
            i = line.idx
            if i < 2 or lines[i-1].strip() == '' and lines[i-2].strip() == '':
                continue
            ok = False
            print(f"{line.location}: heading must be preceeded by two empty lines")
    return ok


def params_mentioned_in(section: str, lines: List[Line]):
    head, sec, tail = split_section(section, lines)
    mentioned = set()
    for line in sec:
        for m in PARAM_PATTERN.finditer(line):
            word = m.group(1) or m.group(2)
            mentioned.add(word)
    return mentioned


def check_usage(lines: List[Line]):
    params = set(p.name for p in PARAMS)
    implementation_specific = set(
        p.name for p in PARAMS if p.descr.startswith('specific to'))
    mentioned_in_interpreting = params_mentioned_in('Interpreting', lines)
    new_in_interpreting = mentioned_in_interpreting - params
    mentioned_in_connecting = params_mentioned_in('Connecting', lines)
    allow_unused = set(['table', 'tableschema', 'fetchsize'])
    never_used = (
        (params - mentioned_in_interpreting - mentioned_in_connecting)
        | (new_in_interpreting - mentioned_in_connecting)
    ) - allow_unused - implementation_specific
    if not never_used:
        return True
    print("The following parameters are never used:", ", ".join(never_used))
    return False


def check():
    lines = read_spec()
    ok = True
    ok = ok and check_params_table(lines)
    ok = ok and check_whitespace(lines)
    ok = ok and check_bad_underscores(lines)
    ok = ok and check_usage(lines)
    return ok


if __name__ == "__main__":
    argparser = argparse.ArgumentParser()
    argparser.add_argument("--dump-table", action="store_true")
    args = argparser.parse_args()
    if args.dump_table:
        for line in dump_table():
            print(line)
    else:
        if not check():
            exit(1)
