#!/usr/bin/python3

from dataclasses import dataclass
from typing import Optional
from unittest import TestCase


@dataclass
class Addr:
    tcp_host: Optional[str]
    port: Optional[int]
    unix_sock: Optional[str]

    def effective_tcp_host(self):
        if self.tcp_host is not None:
            return self.tcp_host
        if self.unix_sock is None:
            return "localhost"
        return None

    def effective_port(self):
        if self.port is not None:
            return self.port
        return 50_000

    def effective_unix_sock(self):
        if self.unix_sock is not None:
            return self.unix_sock
        if self.effective_tcp_host() == "localhost":
            return f".s.{self.effective_port()}"
        return None

    def effective(self):
        u = self.effective_unix_sock()
        h = self.effective_tcp_host()
        p = self.effective_port()
        t = f"{h}:{p}" if h is not None else None
        return (u, t)


def f(tcp_host, port, unix_sock):
    return Addr(tcp_host, port, unix_sock).effective()


class Tests(TestCase):
    def test_simple_cases(self):
        self.assertEqual(f(None, None, None), (".s.50000", "localhost:50000"))

        self.assertEqual(f("localhost", None, None), (".s.50000", "localhost:50000"))

        self.assertEqual(
            f("localhost.localdomain", None, None),
            (None, "localhost.localdomain:50000"),
        )

        self.assertEqual(f("localhost.", None, None), (None, "localhost.:50000"))

        self.assertEqual(
            f("mdb.example.com", None, None), (None, "mdb.example.com:50000")
        )

        self.assertEqual(
            f("mdb.example.com", 12345, None), (None, "mdb.example.com:12345")
        )

        self.assertEqual(f(None, None, "/tmp/x"), ("/tmp/x", None))

        self.assertEqual(f(None, 12345, None), (".s.12345", "localhost:12345"))
