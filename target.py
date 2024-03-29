"""
Utilities for parsing MonetDB URLs
"""
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0.  If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright 1997 - July 2008 CWI, August 2008 - 2016 MonetDB B.V.


from monetparameters import PARAMS
import re
from typing import Callable, Union
from urllib.parse import parse_qsl, urlparse


# Note that 'valid' is not in VIRTUAL:
CORE = set(['tls', 'host', 'port', 'database', 'tableschema', 'table'])
KNOWN = set([
    'tls', 'host', 'port', 'database', 'tableschema', 'table',
    'sock', 'sockdir', 'cert', 'certhash', 'clientkey', 'clientcert',
    'user', 'password', 'language', 'autocommit', 'schema', 'timezone',
    'binary', 'replysize', 'fetchsize', 'maxprefetch']
)
IGNORED = set(['hash', 'debug', 'logfile'])
VIRTUAL = set([
    'connect_scan', 'connect_unix', 'connect_tcp', 'connect_port',
    'connect_tls_verify', 'connect_certhash_digits',
    'connect_binary', 'connect_clientkey', 'connect_clientcert',
])

_BOOLEANS = dict(
    true=True,
    false=False,
    yes=True,
    no=False,
    on=True,
    off=False
)

_DEFAULTS = dict(
            tls=False,
            host="",
            port=-1,
            database="",
            tableschema="",
            table="",
            sock="",
            sockdir="/tmp",
            cert="",
            certhash="",
            clientkey="",
            clientcert="",
            user="monetdb",
            password="monetdb",
            language="sql",
            autocommit=False,
            schema="",
            timezone=None,
            binary="on",
            replysize=None,
            fetchsize=None,
            maxprefetch=None,
        )


def parse_bool(x: Union[str, bool]):
    if isinstance(x, bool):
        return x
    try:
        return _BOOLEANS[x.lower()]
    except KeyError:
        raise ValueError("invalid boolean value")


class urlparam:
    """Bla"""

    field: str
    parser: Callable[[Union[str, any]], any]

    def __init__(self, name, typ, doc):
        self.field = name
        self.parser = (
            str if typ == 'string' or typ == 'path' else
            int if typ == 'integer' else
            parse_bool
        )
        self.__doc__ = doc

    def __get__(self, instance, owner):
        # what is owner?
        return instance._VALUES.get(self.field)

    def __set__(self, instance, value):
        parsed = (self.parser)(value)
        instance._VALUES[self.field] = parsed
        if self.field in instance._TOUCHED:
            instance._TOUCHED[self.field] = True

    def __delete__(self, instance):
        raise Exception("cannot delete url parameter")


class Target:
    """Holds all parameters needed to connect to MonetDB."""
    __slots__ = [
        '_VALUES',
        '_OTHERS',
        '_TOUCHED',
    ]

    def __init__(self):
        self._VALUES = dict(**_DEFAULTS)
        self._OTHERS = {}
        self._TOUCHED = dict(user=False, password=False)

    tls = urlparam('tls', 'bool', 'secure the connection using TLS')
    host = urlparam(
        'host', 'string', 'IP number, domain name or one of the special values `localhost` and `localhost.`')
    port = urlparam('port', 'integer',
                    'TCP port, also used to pick Unix Domain socket path')
    database = urlparam('database', 'string', 'name of database to connect to')
    tableschema = urlparam('tableschema', 'string', 'only used for REMOTE TABLE, otherwise unused')
    table = urlparam('table', 'string', 'only used for REMOTE TABLE, otherwise unused')
    sock = urlparam('sock', 'path', 'path to Unix Domain socket to connect to')
    sockdir = urlparam('sockdir', 'path', 'directory where implicit Unix domain sockets are created')
    cert = urlparam(
        'cert', 'path', 'path to TLS certificate to authenticate server with')
    certhash = urlparam(
        'certhash', 'string', 'hash of server TLS certificate must start with these hex digits; overrides cert')
    clientkey = urlparam(
        'clientkey', 'path', 'path to TLS key (+certs) to authenticate with as client')
    clientcert = urlparam(
        'clientcert', 'path', "path to TLS certs for 'clientkey', if not included there")
    user = urlparam('user', 'string', 'user name to authenticate as')
    password = urlparam('password', 'string', 'password to authenticate with')
    language = urlparam('language', 'string',
                        'for example, "sql", "mal", "msql", "profiler"')
    autocommit = urlparam('autocommit', 'bool', 'initial value of autocommit')
    schema = urlparam('schema', 'string', 'initial schema')
    timezone = urlparam('timezone', 'integer',
                        'client time zone as minutes east of UTC')
    binary = urlparam(
        'binary', 'string', 'whether to use binary result set format (number or bool)')
    replysize = urlparam('replysize', 'integer',
                         'rows beyond this limit are retrieved on demand, <1 means unlimited')
    maxprefetch = urlparam('maxprefetch', 'integer', 'specific to pymonetdb')

    # alias
    fetchsize = replysize

    def set(self, key: str, value: str):
        if key in KNOWN:
            setattr(self, key, value)
        elif key in IGNORED or '_' in key:
            self._OTHERS[key] = value
        else:
            raise ValueError(f"unknown parameter {key!r}")

    def get(self, key: str):
        if key in KNOWN or key in VIRTUAL:
            return getattr(self, key)
        elif key in IGNORED or '_' in key:
            return self._OTHERS[key]
        else:
            raise KeyError(key)

    def boundary(self):
        """If user was set and password wasn't, clear password"""
        if self._TOUCHED['user'] and not self._TOUCHED['password']:
            self.password = ''
        self._TOUCHED['user'] = False
        self._TOUCHED['password'] = False

    def parse(self, url: str):
        self.boundary()
        if url.startswith("monetdb://") or url.startswith("monetdbs://"):
            self._set_core_defaults()
            self._parse_monetdb_url(url)
        elif url.startswith("mapi:monetdb://"):
            self._set_core_defaults()
            self._parse_mapi_monetdb_url(url)
        else:
            raise ValueError("URL must start with monetdb://, monetdbs:// or mapi:monetdb://")
        self.boundary()

    def _set_core_defaults(self):
        self.tls = False
        self.host = ''
        self.port = _DEFAULTS['port']
        self.database = ''

    def _parse_monetdb_url(self, url):
        parsed = urlparse(url, allow_fragments=True)

        if parsed.scheme == 'monetdb':
            self.tls = False
        elif parsed.scheme == 'monetdbs':
            self.tls = True
        else:
            raise ValueError(f"Invalid URL scheme: {parsed.scheme}")

        if parsed.hostname is not None:
            host = strict_percent_decode('host name', parsed.hostname)
            if host == 'localhost':
                host = ''
            elif host == 'localhost.':
                host = 'localhost'
            self.host = host
        if parsed.port is not None:
            port = parsed.port
            if port is not None and not 1 <= port <= 65535:
                raise ValueError(f"Invalid port number: {port}")
            self.port = port

        path = parsed.path
        if path:
            parts = path.split("/")
            # 0: before leading slash, always empty
            # 1: database name
            # 2: schema name, ignored
            # 3: table name, ignored
            # more: error
            assert parts[0] == ""
            if len(parts) > 4:
                raise ValueError("invalid table name: " + '/'.join(parts[3:]))
            self.database = strict_percent_decode('database name', parts[1])
            if len(parts) > 2:
                self.tableschema = strict_percent_decode('schema name', parts[2])
            if len(parts) > 3:
                self.table = strict_percent_decode('table name', parts[3])

        for key, value in parse_qsl(parsed.query, keep_blank_values=True, strict_parsing=True):
            if not key:
                raise ValueError("empty key is not allowed")
            key = strict_percent_decode(repr(key), key)
            value = strict_percent_decode(f"value of {key!r}", value)
            if key in CORE:
                raise ValueError(
                    "key {key!r} is not allowed in the query parameters")
            self.set(key, value)

    def _parse_mapi_monetdb_url(self, url):
        # mapi urls have no percent encoding at all
        parsed = urlparse(url[5:])
        if parsed.scheme != 'monetdb':
            raise ValueError(f"Invalid scheme {parsed.scheme!r}")
        self.tls = False
        if parsed.username is not None:
            self.user = parsed.username
        if parsed.password is not None:
            self.password = parsed.password
        if parsed.hostname is not None:
            self.host = parsed.hostname
        if parsed.port is not None:
            self.port = parsed.port

        path = parsed.path
        if path is not None:
            if parsed.hostname is None and parsed.port is None:
                self.sock = path
            else:
                path = path[1:]
                self.database = path  # validation will happen later

        # parse query manually, the library functions perform percent decoding
        if not parsed.query:
            return
        for part in parsed.query.split('&'):
            # language
            if part.startswith('language='):
                self.language = part[9:]
            elif part.startswith('database='):
                self.database = part[9:]
            elif part.startswith('user=') or part.startswith('password='):
                # ignored because libmapi does so
                pass
            elif part.startswith('binary='):
                # pymonetdb-only, backward compat
                self.binary = part[7:]
            elif part.startswith('replysize='):
                # pymonetdb-only, backward compat
                self.replysize = part[10:]
            elif part.startswith('maxprefetch='):
                # pymonetdb-only, backward compat
                self.maxprefetch = part[12:]
            else:
                # unknown parameters are ignored
                pass


    def validate(self):
        # 1. The parameters have the types listed in the table in [Section
        #    Parameters](#parameters).
        #
        # This has already been checked by the url_param magic.

        # 2. At least one of **sock** and **host** must be empty.
        if self.sock and self.host:
            raise ValueError("With sock=, host must be empty or 'localhost'")

        # 3. The string parameter **binary** must either parse as a boolean or as a
        #    non-negative integer.
        #
        # Let connect_binary do all the work.
        if self.connect_binary < 0:
            raise ValueError("Parameter 'binary' must be ≥ 0")

        # 4. If **sock** is not empty, **tls** must be 'off'.
        if self.sock and self.tls:
            raise ValueError("TLS cannot be used with Unix domain sockets")

        # 5. If **certhash** is not empty, it must be of the form `{sha256}hexdigits`
        #    where hexdigits is a non-empty sequence of 0-9, a-f, A-F and colons.
        if self.certhash and not _HASH_PATTERN.match(self.certhash):
            raise ValueError("invalid certhash")

        # 6. If **tls** is 'off', **cert** and **certhash** must be 'off' as well.
        if not self.tls and (self.cert or self.certhash):
            raise ValueError("'cert' and 'certhash' can only be used with monetdbs:")

        # 7. Parameters **database**, **tableschema** and **table** must consist only of
        #    upper- and lowercase letters, digits, dashes and underscores. They must not
        #    start with a dash.
        if self.database and not _DATABASE_PATTERN.match(self.database):
            raise ValueError(f"invalid database name {self.database!r}")
        if self.tableschema and not _DATABASE_PATTERN.match(self.tableschema):
            raise ValueError(f"invalid schema name {self.tableschema!r}")
        if self.table and not _DATABASE_PATTERN.match(self.table):
            raise ValueError(f"invalid table name {self.table!r}")

        # 8. Parameter **port**, if present, must be in the range 1-65535.
        if self.port != -1 and not 1 <= self.port <= 65535:
            raise ValueError(f"Invalid port number: {self.port}")

        # 9. If **clientcert** is set, **clientkey** must also be set.
        if self.clientcert and not self.clientkey:
            raise ValueError("clientcert can only be used together with clientkey")

    @property
    def connect_scan(self):
        if not self.database:
            return False
        if self.sock or self.host or self.port != -1:
            return False
        if self.tls:
            return False
        return True

    @property
    def connect_unix(self):
        if self.sock:
            return self.sock
        if self.tls:
            return ""
        if self.host == "":
            return f"{self.sockdir}/.s.monetdb.{self.connect_port}"
        return ""

    @property
    def connect_tcp(self):
        if self.sock:
            return ""
        return self.host or "localhost"

    @property
    def connect_port(self):
        assert self.port == -1 or 1 <= self.port <= 65535
        if self.port == -1:
            return 50000
        else:
            return self.port

    @property
    def connect_tls_verify(self):
        if not self.tls:
            return ""
        if self.certhash:
            return "hash"
        if self.cert:
            return "cert"
        return "system"

    @property
    def connect_clientkey(self):
        return self.clientkey

    @property
    def connect_clientcert(self):
        return self.clientcert or self.clientkey

    @property
    def connect_binary(self):
        try:
            return int(self.binary)
        except ValueError:
            try:
                return 65535 if parse_bool(self.binary) else 0
            except ValueError:
                raise ValueError("invalid value for 'binary': {self.binary}, must be int or bool")

    @property
    def connect_certhash_digits(self):
        m = _HASH_PATTERN.match(self.certhash)
        return m.group(1).lower().replace(':', '')

_UNQUOTE_PATTERN = re.compile(b"[%](.?.?)")
_DATABASE_PATTERN = re.compile("^[A-Za-z0-9_][-A-Za-z0-9_.]*$")
_HASH_PATTERN = re.compile(r"^[{]sha256[}]([0-9a-fA-F:]+)$")

def _unquote_fun(m) -> bytes:
    digits = m.group(1)
    if len(digits) != 2:
        raise ValueError()
    return bytes([int(digits, 16)])


def strict_percent_decode(context: str, text: str) -> str:
    try:
        return str(_UNQUOTE_PATTERN.sub(_unquote_fun, bytes(text, "ascii")), "utf-8")
    except (ValueError, UnicodeDecodeError) as e:
        raise ValueError("invalid percent escape in {context}") from e


t = Target()
