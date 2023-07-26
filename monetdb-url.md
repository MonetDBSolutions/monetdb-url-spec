# MonetDB URL Specification

Version: 0.2pre

Status: Proposal

This document defines the meaning of URLs with scheme `monetdb:` and
`monetdbs:`, and reserves scheme `monetdbe:` for later. It also defines an
interpretation for a subset of the classic `mapi:monetdb:` URLs used by libmapi,
mclient and monetdbd, and for the `mapi:monetdb:` variant used by pymonetdb.

We speak of a URL rather than a URI because the purpose of the MonetDB URL is
not just to identify a given MonetDB instance, but in particular to state how to
connect to it. Also, a MonetDB URL is not really suitable as an identifier
because there are many ways to connect to the same database.

The terms MAY, MUST, SHOULD, etc, in this document should be interpreted as
described in [RFC 2119][rfc2119].

[rfc2119]: https://datatracker.ietf.org/doc/html/rfc2119
[rfc3986]: https://datatracker.ietf.org/doc/html/rfc3986

TODO field / property / attribute be consistent


## Examples

<dl>

  <dt><code>monetdb://localhost:50000/demo?user=monetdb&password=monetdb</code></dt>
  <dd>
    First try to connect using Unix Domain socket <code>/tmp/.s.monetdb.50000</code>, if
    supported. If that fails, try a TCP connection to localhost (either IPv6 or
    IPv4) port 50000. Try to log in to database <code>demo</code> using the given user name
    and password. Do not attempt to use TLS.
  </dd>

  <dt><code>monetdb:///demo</code></dt>
  <dd>
    Equivalent of the above, except that user name and password are no longer specified.
  </dd>

  <dt><code>monetdb://localhost.localdomain/demo</code></dt>
  <dd>
    Try to connect to database 'demo' on TCP port 50000 on localhost (either IPv4
    or IPv6). Do not try the Unix Domain socket. Can be abbreviated to
    <code>monetdb://localhost./demo</code>.

    TODO BEFORE 0.2: or can it?
  </dd>

  <dt><code>monetdb://mdb.example.com:12345/demo</code></dt>
  <dd>
    Try to connect to database 'demo' on TCP port 12345 on mdb.example.com.
  </dd>

  <dt><code>monetdb://192.168.13.4:12345/demo</code></dt>
  <dd>
    Try to connect to database 'demo' on TCP
    port 12345 on the given IPv4 address.
  </dd>

  <dt><code>monetdb://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:12345/demo</code></dt>
  <dd>
    Try to connect to database 'demo' on TCP port 12345 on the given IPv6 address.
  </dd>

  <dt><code>monetdb://localhost/</code></dt>
  <dd>
    Try to connect to an unspecified database on <code>/tmp/.s.monetdb.50000</code>
    or if that fails, TCP port 50000 on localhost. This will only work when
    connecting to a raw mserver5.
  </dd>

  <dt><code>monetdbs://mdb.example.com/demo</code></dt>
  <dd>
    (Note the 's' in <code>monetdbs</code>.)
    Similar to <code>monetdb://mdb.example.com/demo</code>, but secure the connection
    using TLS. Use root certificates from the system trusted certificate store to
    authenticate the server.
  </dd>

  <dt><code>monetdbs://mdb.example.com/demo?cert=/home/user/server.crt</code></dt>
  <dd>
    Connect to mdb.example.com. Secure the connection with TLS (used to be called SSL).
    Authenticate the server against the TLS certificate found in file /home/user/server.crt.
    Fail if the certificate is not present in the indicated location on the client host.
  </dd>

  <dt><code>monetdb:///demo?sock=/tmp/.s.monet.50000&user=dbuser</code></dt>
  <dd>
    Connect using the given Unix Domain socket in the file system. Do not use TCP.
    This syntax departs from the old URL syntax, where it would be
    <code>mapi:monet:///tmp/.s.monetdb.50000?database=demo</code>.
  </dd>

  <dt><code>monetdbe:///path/to/database/directory</code></dt>
  <dd>
    Run the embedded version of MonetDB inside the client and open the given path
    with it. This is a dbpath, so the given directory would be expected to
    contain subdirectories <code>bat/</code> and <code>sql_logs/</code>.

    Note: <code>monetdbe:</code> URLs are mentioned but not specified in this
    version of this document.
  </dd>

</dl>


## Syntax

The URL is parsed according to the rules in [RFC 3986][rfc3986]. The general form is:

<pre>
 monetdb://[<b>host</b>[:<b>port</b>]]/[<b>database</b>[/<b>table_schema</b>[/<b>table</b>]]][?param1=value1[&param2=value2...]]
monetdbs://[<b>host</b>[:<b>port</b>]]/[<b>database</b>[/<b>table_schema</b>[/<b>table</b>]]][?param1=value1[&param2=value2...]]
monetdbe:/to/be/determined
</pre>

TODO BEFORE soon: storing information is an implementation, not a specification.
"it will be convenient to assume all extracted information is stored in a
*connection record* which has a field for each supported query parameter,
plus some additional fields"

The extracted information is stored in a *connection record* containing a large
number of fields, all optional. The following fields are extracted from the
"main part" of the URL and are not permitted as query parameters:

| Field            | Type | Remark                                             |
| ---------------- | ---- | -------------------------------------------------- |
| **use_tls**      | bool | true if scheme is `monetdbs:`, false if `monetdb:` |
| **host**         | str  | `localhost` has special meaning                    |
| **port**         | int  |                                                    |
| **database**     | str  |                                                    |
| **table_schema** | str  | only used for REMOTE TABLE, otherwise ignored      |
| **table**        | str  | only used for REMOTE TABLE, otherwise ignored      |

The rest of the fields come from the query parameters:

| Field           | Type     | Remark                                                   |
| --------------- | -------- | -------------------------------------------------------- |
| **sock**        | path     | path to Unix Domain socket to connect to                 |
| **cert**        | path     | path to TLS certificate to authenticate server with      |
| **clientkey**   | path     | path to TLS key (+certs) to authenticate with as client  |
| **clientcert**  | path     | path to TLS certs for 'clientkey', if not included there |
| **user**        | str      |                                                          |
| **password**    | str      |                                                          |
| **language**    | str      | only 'sql' is important                                  |
| **autocommit**  | bool     |                                                          |
| **schema**      | str      |                                                          |
| **timezone**    | int      | as minutes east of UTC                                   |
| **replysize**   | int      |                                                          |
| **fetchsize**   | int      | alias for 'replysize', needed for JDBC                   |
| **maxprefetch** | int      |                                                          |
| **binary**      | bool/int | whether to use binary result set format                  |
| **debug**       | bool     | meaning is implementation specific                       |
| **logfile**     | str      | meaning is implementation specific                       |

Booleans can be written as true/false, yes/no or on/off.

Any query parameter not in this table MUST be rejected with an error, except if
the name contains an underscore `_`. Query parameters with an underscore are
implementation specific and MUST be ignored if not recognized. For example, the
JDBC driver recognizes `so_timeout`, `treat_clob_as_varchar` and
`treat_blob_as_binary`, which may be ignored by other implementations.


### Parsing classic mapi:monetdb: URLs

TODO move this down!

Every implementation also needs to be able to deal with the classic
`mapi:monetdb:` URLs because they may occur in monetdbd's redirect messages
while logging in. They can also occur in the output of `monetdb status`.

Note: pymonetdb supports an extended `mapi:monetdb:` syntax which includes user
name, password, and a few more options:

    mapi:monetdb://user:password@host:port/database

However, this extension is specific to pymonetdb and other implementations
SHOULD NOT support it.

This is how the classic URLs should be interpreted in terms of a connection
record:

* The URL starts with `mapi:monetdb://`. There is no `mapi:monetdbs:` variant and
  there never will be. The prefix `mapi:merovingian:` is used internally in the
  MAPI protocol but it is never visible to users and it is not governed by this
  specification.

* No percent decoding is performed anywhere, at all.

* Host name and port are passed in the usual way after the double slashes.

* If a host name is given, the path component of the URL is interpreted as the
  database name.

* If no host name is given, that is, if the URL starts with `mapi:monetdb:///`,
  the path component is interpreted as a file system path to a Unix Domain socket
  to connect to.

* Only two query parameters are recognized: `language` and `database`.
  The latter is only only allowed with Unix Domain sockets, that is,
  triple-slash URLs.

* All other parameters MUST be ignored.


## Combining connection records

When information from multiple sources is combined, for example command line
parameters AND a URL, the following rules should be observed:

In general, if we have an "old" and a "new" connection record, fields present in
the new record override those in the old record. However,

Rule **U1**: if **user** is changed, that is, if the **user** field is present
in both the old and the new record and the values are not equal, the **password**
field must be cleared unless it's present in the new record.

Rule **U2**, which applies ONLY if the new record comes from a URL: if any of the
fields **host**, **port**, **sock** or **database** are present in the new
record, the ones that are not present must be cleared.


## Validation rules

Before trying to connect, the implementation must verify that the connection
record satisfies the following constraints:

> Rule **V1**: if **sock** and **host** are both present, **host** must be
> equal to `localhost`.

Unix Domain sockets do not make sense when connecting to any other host than
the local host.

> Rule **V2**: if present, **port** must be in the range 1-65535 (inclusive).

In particular, it must not be 0 or negative.

> Rule **V3**: if **clientcert** is present, **clientkey** must be present as
> well.

The other way around is allowed, as the certificates can be included in the
key file.

> Rule **V4**: if **password** is present, **user** must be present as well.

It doesn't make any sense otherwise.

> Rule **V5**: all fields of string type, if present, must not be the empty
> string. Exceptions:
>
> 1. **password** is allowed to be the empty string.


## Effective attributes

Before the connection can be established, a number of decisions needs to be
made. For example, we need to set default values, and we need to decide whether
an attempt should be made to connect to a Unix Domain socket before trying a
TCP connection.

These decisions are expressed as a number of *effective attributes* that are
derived from other fields. For example, **effective_unix_sock** is derived from
fields **sock**, **host** and **effective_port**.

<dl>

  <dt>effective_use_tls</dt>
  <dd>
    the value of <b>use_tls</b> if present;
    false otherwise.
  </dd>

  <dt>effective_tcp_host</dt>
  <dd>
    the value of <b>host</b> if present;
    not present if <b>sock</b> is present;
    `localhost` if neither <b>host</b> nor <b>sock</b> are present.
  </dd>

  <dt>effective_port</dt>
  <dd>
    the value of <b>port</b>, if present;
    50_000 otherwise.
  </dd>

  <dt>effective_unix_sock</dt>
  <dd>
    the value of <b>sock</b> if present;
    <code>/tmp/.s.monetdb.<b>effective_port</b></code> if <b>host</b> is <code>localhost</code> or not present;
    not present otherwise.
  </dd>

  <dt>effective_language</dt>
  <dd>
    the value of <b>language</b> if present;
    <code>sql</code> otherwise.
  </dd>

  <dt>effective_replysize</dt>
  <dd>
    the value of <b>replysize</b>, if present;
    the value of <b>fetchsize</b>, if present;
    not present otherwise.
  </dd>

  <dt>effective_binary</dt>
  <dd>
    the minimum of <b>binary</b> and the maximum supported level, if <b>binary</b> is a number;
    0, if <b>binary</b> is False;
    the maximum supported level, if <b>binary</b> is True.
  </dd>

</dl>


## Establishing the connection

1. If Unix Domain sockets are supported, try to connect to
   **effective_unix_sock**. If this succeeds, move on to step 4. If it fails,
   remember the error and move to the next step. If Unix Domain sockets are not
   supported, remember an error that says so.

2. If **effective_tcp_host** is present, connect to **effective_tcp_host** at
   port **effective_port**. If this succeeds move on to step 4. If it fails,
   remember the error.

3. If you get here, abort the connection attempt with the remembered error.

4. The connection has been established.
   If **effective_use_tls** is True, wrap it in TLS using
   **tls_cert**, **client_key** and **client_cert**. Abort if this fails.

5. Perform a MAPI login using **user**, **password**, **database** and
   **effective_language**. Configure the session using **autocommit**,
   **default_schema**, **timezone** and **reply_size**, using sensible default
   values if not present.

6. During the course of the session, use the binary result set protocol when
   appropriate, depending on **effective_binary** and the capabilities of the
   implementation and the server.


## Implementation in JDBC

A JDBC URL always starts with `jdbc:`. For MonetDB, that prefix is followed by a
MonetDB URL as specified in this document. The URL format specified in this
document is intended to be backward compatible with the existing JDBC driver in
the sense that all existing `jdbc:monetdb:` URLs ought to have the same meaning
when interpreted under the new rules. However, the new scheme offers more
options and, of course, TLS support.

Note: the JDBC API allows to pass a `Properties` object together with the URL.
All query parameters can also be passed as property with the same name. The
information in the URL and the information in the properties object is combined
according to the rules in
Section&nbsp;[Combining connectionrecords](#combining-connection-records),
with the properties as the 'old' record and the URL as the 'new' record.

As described in Section&nbsp;[Syntax](#syntax), the fields **use_tls**,
**host**, **port** and **database** cannot be passed as query parameters and
therefore also not in the Properties object. However, as an exception, the JDBC
driver will not apply this rule if the full URL is exactly equal to
`jdbc:monetdb:`, that is, without slashes or anything else. In that case, all
fields can be set through the properties object.

This is convenient because the bare `jdbc:monetdb:` URL can be used with the
return value of the non-standard `getConnectionProperties()` method of the
`MonetConnection` class. This method which returns a properties object with all
information necessary to establish a new, identical connection, including
properties for host, port, etc.


## Implementation in pymonetdb

Pymonetdb currently only uses `mapi:monetdb:` URLs so support for
`monetdb:` and `monetdbs:` URLs can be added while remaining backward compatible.

In the `mapi:monetdb:` URLs, pymonetdb also allows user- and password fields in
the main URL, as in:

    mapi:monetdb://user:password@host:port/database

This capability will be retained but not ported to any other implementations.


## Implementation in libmapi / mclient / ODBC

TODO need to flesh this out more.

We will ensure all command-line tools allow passing a URL instead of the
database name. This is currently the case for mclient and probably for msqldump,
but there may be others.  The interaction between the URL parameter and the
other parameters should be governed by the rules in
Section&nbsp;[Combining connection records](#combining-connection-records).
This is not expected to be a problem because those rules are modelled on how
mclient does it, and if the other tools differ from mclient they should probably
be changed anyway.

Mtest uses separate environment variables `HOST`, `MAPIPORT`, etc., to pass
connection information to sqllogictest.py and other test scripts. Some of the
test scripts call still other programs such as mclient. It would be good if we
could run at least a substantial portion of the test suite against TLS-protected
servers. Tests that start their own mservers can obviously not be included but
most of the bread-and-butter tests can. To achieve this, Mtest should provide
a MAPIURL variable and most of the other tooling should pass that around as-is.
Details to be investigated.

MonetDBD should be extended with a base-url to be used for the `monetdb status`
output and in MAPI redirects. Example...

Libmapi, function `mapi_mapiuri` must accept the new URL styles.
There is also `mapi_get_uri`, what should it return?
classic url? new style url? should it include username and password? only
user name?  all have valid use cases.  introduce new function with more settings,
make mapi_get_uri a classic-only wrapper.  but what should the wrapper return
if cannot be represented as classic (e.g., monetdbs?).
probably also long range of get_/set_field functions.

ODBC listens to MAPIPORT, sounds like a bad idea to me.

