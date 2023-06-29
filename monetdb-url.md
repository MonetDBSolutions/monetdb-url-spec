# MonetDB URL Specification

Version: 0.1pre

Status: Proposal

This document defines the meaning of [URLs][rfc3986] with scheme `monetdb:` and `monetdbs:`
and reserves scheme `monetdbe:` for a future version of the specification. It
also defines an interpretation for a subset of the classic `mapi:monetdb:`
URLs.

We speak of a URL rather than a URI because the purpose of the MonetDB URL is not
just to identify a given MonetDB, but especially to state how to connect to it.
Also, it is not suitable as an identifier because there are many URL's that
connect to the same database.

The terms MAY, MUST, SHOULD, etc, in this document should be interpreted as
described in [RFC2119][rfc2119].

TODO BEFORE 0.2: actually write in the style of RFC2119.

When we speak of 'an implementation' or 'the implementation' we mean a MonetDB
client such as mclient/libmapi, pymonetdb or monetdb-jdbc.

[rfc2119]: https://datatracker.ietf.org/doc/html/rfc2119
[rfc3986]: https://datatracker.ietf.org/doc/html/rfc3986


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
  Can be abbreviated further to <code>monetdb:demo</code>.

  TODO BEFORE 0.2: is this abbreviation a good idea? If so, what are the exact rules?
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


## Overview

The semantics of MonetDB URLs are defined in terms of an intermediate stage, the
*Connection Record*. A connection record contains all information necessary to
establish a connection to MonetDB. It consists of a number of properties defined
in section [Connection Record](#connection-record). Not all combinations of
properties are valid, the validity rules are also listed there.

Before the connection can be established, a number of rules must be applied
first. For example, if no port number has been supplied, port 50000 is assumed.
As another example, if no explict Unix Domain socket has been supplied and the host name is still
set to the default, the implementation will first attempt to connect to Unix
Domain socket <code>/tmp/.s.monetdb.<b>portnumber</b></code> before attempting
to make a TCP connection to localhost.
In section [Connecting to MonetDB](#connecting-to-monetdb), these rules are
expressed as derivations of so-called "effective properties" from other
properties. For example, **effective_unix_sock** is derived from the values of
**unix_sock**, **tcp_host** and **effective_port**.

One advantage of defining the semantics through the Connection Record is that it
allows us to express how information from multiple sources can be combined. For
example, the exact meaning of the command line parameters of `mclient` can also
be expressed in terms of a connection record, and then we can state
unambiguously for example what is supposed to happen when `mclient` receives
both a `monetdb:` URL and a separate hostname on the command line.

Another advantage is that it allows us to provide unit tests with specification.
The unit tests can test URL parsing, connection record manipulation and the
computation of the effective properties. These are a lot of test cases that
can then be shared among all implementations.


## Connection Record

A connection record contains the following properties. Not all properties need
to be present. Or, equivalently, some or all properties may contain a null
value. A connection record where all properties are not present (are null) is
called an *empty connection record*.

This section prescribes three *validity rules*, which define which combinations
of properties are valid, and two *update rules*, which must be followed when
altering a connection record.

These are the properties in a connection record:

* The properties **tcp_host**, **port** and **unix_sock** indicate where to
  connect to. If property **tcp_host** is present, it can be either a DNS name, an
  IPv4 address or an IPv6 address. If it contains at least one colon `:`, it
  must be considered an IPv6 address. If it consists of four decimal numbers
  separated by periods `.`, it must be considered an IPv4 address. Otherwise, it
  must be considered a host name to be resolved using the system resolver.
  If property **port** is present, it must be a number in the range 1-65535. If
  property **unix_sock** is present, it is the path to a Unix Domain socket to
  connect to.

Note: the interpretation of the **tcp_host** field is different from the `host`
component of the URL. See section [Connecting to MonetDB](#connecting-to-monetdb) for details.

TODO BEFORE 0.1: make sure that the above is actually explained in the section referred to.

TODO BEFORE 0.1: refer to [RFC 3986 section
3.2.2](https://datatracker.ietf.org/doc/html/rfc3986#section-3.2.2).


Validity rule **V1**: all three properties **tcp_host**, **port** and
**unix_sock** may be omitted but if **unix_sock** is present, neither
**tcp_host** nor **port** must be present.

Update rule **U1**: all three properties **tcp_host**, **port** and
**unix_sock** can be set or cleared individually, for example based on command
line parameters. However, when (and only when) updating a connection record with
information *from a URL*, they must be cleared if the URL does not set a value
for them. For example, suppose **port** was originally set to 12345. After the
URL `monetdb://dbhost/demo` has been parsed, **port** will no longer be present
in the connection record.

* Property **use_tls** is a boolean that indicates whether to encrypt the
  connection and authenticate the server using TLS.  Property **tls_cert** is
  the path on the client to a certificate to be used to authenticate the server.
  If not present, the root certificates built into the client OS or Runtime
  should be used. Property **client_key** is the path to a private key to be
  used to authenticate the client to the server. Property **client_cert** is the
  path to a certificate chain for that key. However, note that in section
  [Connecting to MonetDB](#connecting-to-monetdb), **effective_client_cert**
  defaults to **client_key** if present, to allow for situations where the
  certificates are bundled with the key.

TODO BEFORE 0.2: should there be a validity rule that says that **tls_cert**,
**client_key** and **client_cert** can only be present if **use_tls** is set to
True?

* Property **dbpath** is reserved for use with `monetdbe:` URLs.

Validity rule **V2**: if **dbpath** is present, none of **tcp_host**, **port**,
**unix_sock** and **use_tls** can be present.

* Property **database** indicates the name of the database to connect to. It is
  only needed when connecting to `monetdbd`, not when connecting directly to
  `mserver5`. The properties **table_schema** and **table** can be used in
  `REMOTE TABLE` definitions but are not used while connecting.

* The properties **user** and **password** are used when authenticating to MonetDB.

Validity rule **V3**: if **password** is present, **user** must also be present.

Update rule **U2**: if the **user** property is updated, either through an URL
or through other means, **password** MUST be cleared unless a new password
is provided simultaneously.

Security note: including the password in the URL is convenient but insecure.
Implementions should also provide another mechanism to enter the password and
possibly the user name into the connection record.

* The property **language** is used when logging into MonetDB. Currently known
  values are `sql`, `mal`, `msql` and `profiler`. Implementations are not required
  to support all of them.

TODO BEFORE 0.2: use the phrase "valid but may cause an error at connect time"
more often.

* The properties **autocommit**, **default_schema**, **timezone**,
  **reply_size** and **binary** are used to override default settings after
  connection has been established. Property **autocommit** is a boolean,
  **default_schema** is a string. Property **timezone** is a signed integer
  indicating the number of minutes EAST of UTC. Property **reply_size** is the
  default number of rows of a result set to include in the initial response to a
  query. The other rows will be postponed until the client asks for them. Values
  0 and -1 mean "no limit".

  Property **binary** is a nonnegative number or a boolean. Numbers indicate
  various levels of support for the binary result set format. At the time of
  writing, levels 0 and 1 are in use. The boolean values are aliases of numbers:
  False corresponds to 0 and True corresponds to the highest value supported by
  the implementation. This should be resolved at connect time, not in the
  connection record. In other words, with the URL `monetdb:///demo?binary=yes`,
  the property should be set to True and not to a number.

  Property **maxprefetch** indicates an upper bound on the number of rows the
  implementation fetches ahead of time. This is done by for example pymonetdb
  and monetdb-java. Implementations that do not fetch ahead can ignore it.

* Implementations may extend the connection record with additional properties
  that only they recognize. Except for a limited number of backward
  compatibility exceptions, the names of such properties and the corresponding
  URL parameters should start with letter 'x'.  Implementations must store all
  such properties even if they do not understand them, and pass them on if the
  connection record is converted into a new URL.


## Connecting to MonetDB

Before connecting, the final value of a number of settings needs to be determined.

<dl>

<dt>effective_tcp_host</dt>
<dd>
  <b>tcp_host</b> if present;
  <code>localhost</code> if <b>unix_sock</b> is not present;
  not present otherwise.
</dd>

<dt>effective_port</dt>
<dd>
  <b>port</b> if present;
  50_000 otherwise.
</dd>

<dt>effective_unix_sock</dt>
<dd>
  <b>unix_sock</b>, if present;
  <code>/tmp/.s.monetdb.<b>&lt;effective_port&gt;</b></code> if <b>effective_tcp_host</b> is 'localhost';
  not present otherwise.
</dd>

<dt>effective_use_tls</dt>
<dd>
  **use_tls**, if present;
  False, otherwise.
</dd>

<dt>effective_database</dt>
<dd>
  <b>database</b>, if present;
  the empty string, otherwise.
</dd>

<dt>effective_language</dt>
<dd>
  <b>language</b> if present;
  <code>sql</code> otherwise.
  If language <b>effective_language</b> is not supported, abort.
</dd>

<dt>effective_binary</dt>
<dd>
  the minimum of <b>binary</b> and the maximum supported level, if <b>binary</b> is a number;
  0, if <b>binary</b> is False;
  the maximum supported level, if <b>binary</b> is True.
</dd>

</dl>

With these values determined, the connection is set up as follows:

1. This version of the specification does not cover the situation where
   **dbpath** is present. In the remainder of this section, assume it is not
   present.

2. If Unix Domain sockets are supported, try to connect to
   **effective_unix_sock**. If this succeeds, move on to step 5. If it fails,
   remember the error and move to the next step. If Unix Domain sockets are not
   supported, remember an error that says so.

3. If **effective_tcp_host** is present, connect to **effective_tcp_host** at
   port **effective_port**. If this succeeds move on to step 5. If it fails,
   remember the error.

4. If you get here, abort the connection attempt with the remembered error.

5. If **effective_use_tls** is True, wrap the connection in TLS using
   **tls_cert**, **client_key** and **client_cert**. Abort if this fails.

6. Perform a MAPI login using **effective_database**, **effective_language**,
   **user** and **password**. Configure the session using **autocommit**,
   **default_schema**, **timezone** and **reply_size**, using sensible default
   values if not present.

7. During the course of the session, use the binary result set protocol when
   appropriate, depending on **effective_binary** and the capabilities of the
   implementation and the server.


## Parsing the URL

TODO BEFORE 0.2: Pick section names without a verb, this is a spec not a recipe

The general form of a MonetDB URL is:

<pre>
 monetdb://[<b>host</b>][:<b>port</b>]/[<b>database</b>[/<b>table_schema</b>[/<b>table</b>]]][?param1=value1&[param2=value2...]]
monetdbs://[<b>host</b>][:<b>port</b>]/[<b>database</b>[/<b>table_schema</b>[/<b>table</b>]]][?param1=value1&[param2=value2...]]
monetdbe:/to/be/determined
</pre>

The URL is parsed into components according to the rules in [RFC 3986][rfc3986].
We will use a monospaced font for URL components, e.g., `host, and continue to
use bold face for connection record properties, e.g., **tcp_host**.

TODO BEFORE 0.2: explicitly list the characters that may appear in a URL without
percent-encoding, or refer to the relevant phrase in the [RFC][rfc3986].
Note: we should allow the unescaped colons `:` in query parameters to facilitate
things like `monetdbs://dbhost/demo?cert=C:\Certs\mycert.crt`. Pity we can't allow
spaces.

TODO BEFORE 0.2: are all components percent-decoded? For example, is
`m%6Fnetdb:///demo` a valid URL?

If the scheme is `monetdb:`, **use_tls** is set to False. If it is `monetdbs:`,
**use_tls** is set to True.

Property **tcp_host** is set to component `host` if present, cleared otherwise.
There are two exceptions: if the `host` component is equal to `localhost`, the
**tcp_host** property is cleared. If the `host` component is equal to `localhost.`
(with a trailing period), **tcp_host* is set to `localhost`.

The reason for the above rule is that [RFC 3986][rfc3986] does not permit
the host to be omitted if a port is given. Therefore we map `localhost` to
"**tcp_host** not present" which means "try Unix Domain socket first", and
allow `localhost.` for the rare situation where we actually want to skip the
Unix Domain socket and only try TCP.

Property **port** is set to the value of component `port` if present and
in range, cleared otherwise. Out of range `port` values are an error.

Property **unix_sock** is cleared unless query parameter `sock` is present.

Together, the three rules above implement update rule **U1**.

Property **database** is set to the value of component `database`, if present.

TODO BEFORE 0.1: modify update rule **U1** to also clear **database**.
Add a rule to clear **table_schema** and **table** if **database** is set.
(But only if from a URL?)

### Query Parameters

The query parameters are mapped to properties according to the following table.
Note that the parameter names in the URL are not always identical to the
corresponding connection record property.

if query parameter `user` is present and `password` is not, the **password**
property of the connection record must be cleared. This implements update rule
**U2**.

If a property occurs more than once in the query parameters, all but the last
instance are ignored.

Note that some properties correspond to by more than one query parameter. For
example, both `replysize` and `fetchsize` map to property **replysize**. With
the URL `monetdb:///demo?fetchsize=100&replysize=200&fetchsize=300`,
**replysize** ends up with value 300.

| Query Param   | Property           | Remark                                                      |
| ------------- | ------------------ | ----------------------------------------------------------- |
| `autocommit`  | **autocommit**     | false,no,true,yes (case insensitive)                        |
| `binary`      | **binary**         | NUMBER,false,no,true,yes (case insensitive)                 |
| `cert`        | **tls_cert**       | path on client                                              |
| `clientcert`  | **client_cert**    | path on client                                              |
| `clientkey`   | **client_key**     | path on client                                              |
| `fetchsize`   | **reply_size**     | alias for `replysize`, for the benefit of JDBC              |
| `language`    | **language**       |                                                             |
| `maxprefetch` | **maxprefetch**    |                                                             |
| `password`    | **password**       |                                                             |
| `replysize`   | **reply_size**     |                                                             |
| `schema`      | **default_schema** |                                                             |
| `sock`        | **unix_sock**      | path on client                                              |
| `timezone`    | **timezone**       |                                                             |
| `user`        | **user**           | must clear **password** unless also given                   |
| `username`    | **user**           | alias for `user`; must clear **password** unless also given |
| `xWHATEVER`   | **xWHATEVER**      | implementation-specific                                     |
| `debug`       | **xdebug**         | backward-compatibility alias for `xdebug`                   |
| `logfile`     | **xlogfile**       | backward-compatibility alias for `xlogfile`                 |

TODO BEFORE 0.2: should `maxprefetch` actually just be `prefetch`? Or should we
just make it pymonetdb-specific xmaxprefetch or xprefetch? Note that JDBC also
performs prefetch but the amount is currently not configurable.

TODO BEFORE 0.2: various drivers have timeout parameters. Interpretation varies,
sometimes only on connect, sometimes also on all socket reads and writes. Should
we introduce distinct timeout properties for these cases? Or should we just
specify that, for example, parameter `so_timeout` is defined to be an alias for
`xso_timeout` and leave the possibly inconsistent interpretations to the
individual implementation?


## Implications for JDBC URLs

TODO BEFORE 0.2: move this to a companion document


## Classic MAPI URLs

TODO BEFORE 0.2: move this to a companion document


## Implications for pymonetdb

TODO BEFORE 0.2: move this to a companion document


## Implication for libmapi

TODO BEFORE 0.2: move this to a companion document
