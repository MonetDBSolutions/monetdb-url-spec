#!/usr/bin/env python3


from dataclasses import KW_ONLY, dataclass


@dataclass
class Param:
    name: str
    typ: str
    _: KW_ONLY
    default: str = None
    descr: str
    core: bool = False


PARAMS = [

    Param("tls", "bool", default="false",
          descr="secure the connection using TLS",
          core=True),

    Param("host", "string", default='""',
          descr="IP number, domain name or one of the special values `localhost` and `localhost.`",
          core=True),

    Param("port", "integer", default="50000",
          descr="TCP port, also used to pick Unix Domain socket path",
          core=True),

    Param("database", "string", default='""',
          descr="name of database to connect to",
          core=True),

    Param("tableschema", "string", default='""',
          descr="only used for REMOTE TABLE, otherwise unused",
          core=True),

    Param("table", "string", default='""',
          descr="only used for REMOTE TABLE, otherwise unused",
          core=True),

    Param("sock", "path", default='""',
          descr="path to Unix Domain socket to connect to"),

    Param("cert", "path", default='""',
          descr="path to TLS certificate to authenticate server with"),

    Param("certhash", "string", default='""',
          descr="hash of server TLS certificate must start with these hex digits; overrides cert"),

    Param("clientkey", "path", default='""',
          descr="path to TLS key (+certs) to authenticate with as client"),

    Param("clientcert", "path", default='""',
          descr="path to TLS certs for 'clientkey', if not included there"),

    Param("user", "string", default='unspecified',
          descr="user name to authenticate as"),

    Param("password", "string", default='unspecified',
          descr="password to authenticate with"),

    Param("language", "string", default='"sql"',
          descr='for example, "sql", "mal", "msql", "profiler"'),

    Param("autocommit", "bool", default="unspecified",
          descr="initial value of autocommit"),

    Param("schema", "string", default='""',
          descr="initial schema"),

    Param("timezone", "integer", default="unspecified",
          descr="client time zone as minutes east of UTC"),

    Param("binary", "string", default='"on"',
          descr="whether to use binary result set format (number or bool)"),

    Param("replysize", "integer", default="unspecified",
          descr="rows beyond this limit are retrieved on demand, <1 means unlimited"),

    Param("fetchsize", "integer",
          descr="alias for replysize, specific to jdbc"),

    Param("maxprefetch", "integer", default="unspecified",
          descr="specific to pymonetdb"),

    Param("hash", "string", default="unspecified",
          descr="specific to jdbc"),

    Param("debug", "bool", default="unspecified",
          descr="specific to jdbc"),

    Param("logfile", "string", default="unspecified",
          descr="specific to jdbc"),
]
