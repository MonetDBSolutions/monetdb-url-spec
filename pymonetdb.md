# Implementation in pymonetdb

Pymonetdb currently only uses `mapi:monetdb:` URLs so support for
`monetdb:` and `monetdbs:` URLs can be added while remaining backward compatible.

In the `mapi:monetdb:` URLs, pymonetdb also allows user- and password fields in
the main URL, as in:

    mapi:monetdb://user:password@host:port/database

This capability will be retained but not ported to any other implementations.

