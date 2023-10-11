#!/usr/bin/env python3

import argparse
import os
import re
import sys
from typing import List, Tuple
from unittest import TestCase
import unittest

from target import IGNORED, KNOWN, VIRTUAL, Target, parse_bool


class Line(str):
    """A Line is a string that remembers which file and line number it came from"""
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
        return self.file + ":" + str(self.nr)


def read_lines(f, filename: str, start_line=0) -> List[Line]:
    """Read from 'f' and turn the lines into Lines"""
    n = start_line
    lines = []
    for s in f:
        s = s.rstrip()
        line = Line(s, filename, n)
        lines.append(line)
        n += 1
    return lines


def split_tests(lines: List[Line]) -> List[Tuple[str, List[Line]]]:
    tests = []
    cur = None
    header = None
    count = 0
    location = None
    for line in lines:
        if cur is None:
            if line.startswith("```test"):
                location = line.location
                cur = []
            elif line.startswith('#'):
                header = line.lstrip('#').strip()
                header = re.sub(r'\W+', '_', header).lower()
                count = 0
        else:
            if line.startswith("```"):
                count += 1
                name = f"{header}_{count}"
                assert len(cur) > 0
                tests.append((name, cur))
                cur = None
            else:
                cur.append(line)
    if cur is not None:
        raise Exception(f"Unclosed block at {location}")
    return tests


class TargetTests(TestCase):

    def test_generic_tests(self):
        filename = "tests.md"
        self.run_test_script("tests.md")

    def test_dummy_tests(self):
        filename = 't.md'
        if not os.path.exists(filename):
            raise unittest.SkipTest(f"{filename} does not exist")
        self.run_test_script(filename)


    def run_test_script(self, filename):
        lines = read_lines(open(filename), filename)
        tests = split_tests(lines)
        for name, test in tests:
            start = test[0].location
            print(name, start, file=sys.stderr)
            with self.subTest("joeri", name=name, location=test[0].location):
                self.run_test(test)

    def run_test(self, test):
        target = Target()
        for line in test:
            try:
                self.apply_line(target, line)
                continue
            except AssertionError as e:
                if hasattr(e, 'add_note'):
                    e.add_note(f"At {line.location}")
                    raise
                else:
                    raise AssertionError(f"At {line.location}: {e}")
            except unittest.SkipTest:
                break
            except Exception as e:
                if hasattr(e, 'add_note'):
                    e.add_note(f"At {line.location}")
                    raise

    def apply_line(self, target: Target, line: Line):
        if not line:
            return

        command, rest = line.split(None, 1)
        command = command.upper()
        if command == "PARSE":
            self.apply_parse(target, rest)
        elif command == "ACCEPT":
            self.apply_accept(target, rest)
        elif command == "REJECT":
            self.apply_reject(target, rest)
        elif command == "EXPECT":
            key, value = rest.split('=', 1)
            self.apply_expect(target, key, value)
        elif command == "SET":
            key, value = rest.split('=', 1)
            self.apply_set(target, key, value)
        elif command == "ONLY":
            impl = rest
            if impl != 'pymonetdb':
                raise unittest.SkipTest(f"only for {impl}")
        elif command == "NOT":
            impl = rest
            if impl == 'pymonetdb':
                raise unittest.SkipTest(f"not for {impl}")
        else:
            self.fail(f"Unknown command: {command}")

    def apply_parse(self, target: Target, url):
        target.parse(url)

    def apply_accept(self, target: Target, url):
        target.parse(url)
        target.validate()

    def apply_reject(self, target: Target, url):
        try:
            target.parse(url)
        except ValueError:
            return
        # last hope
        try:
            target.validate()
        except ValueError:
            return
        raise ValueError("Expected URL to be rejected")

    def apply_set(self, target: Target, key, value):
        target.set(key, value)

    def apply_expect(self, target: Target, key, expected_value):
        if key == 'valid':
            should_succeed = parse_bool(expected_value)
            try:
                target.validate()
                if not should_succeed:
                    self.fail("Expected valid=false")
            except ValueError as e:
                if should_succeed:
                    self.fail(f"Expected valid=true, got error {e}")
            return

        if key in VIRTUAL:
            target.validate()

        actual_value = target.get(key)
        if isinstance(actual_value, bool):
            expected_value = parse_bool(expected_value)
        elif isinstance(actual_value, int):
            try:
                expected_value = int(expected_value)
            except ValueError:
                # will show up in the comparison below
                pass
        if actual_value != expected_value:
            self.fail(f"Expected {key}={expected_value!r}, found {actual_value!r}")
