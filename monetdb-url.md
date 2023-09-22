# MonetDB URL Specification

Version: 0.3pre1

Status: Proposal

This document defines the meaning of URLs with scheme `monetdb:` and
`monetdbs:`, and reserves scheme `monetdbe:` for later. It also defines an
interpretation for a common subset of the classic `mapi:monetdb:` URLs used by
libmapi, mclient and monetdbd, and for the `mapi:monetdb:` variant used by
pymonetdb.

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
    Equivalent of the above, except that user name and password are no longer specified
    and must be provided externally.
  </dd>

  <dt><code>monetdb://localhost./demo</code></dt>
  <dd>
    Try to connect to database 'demo' on TCP port 50000 on localhost (either IPv4
    or IPv6). Do not try the Unix Domain socket.

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
    or if that fails, TCP port 50000 on localhost. Because no database name is given,
    this will only work when to a raw mserver5.
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
    Connect to mdb.example.com. 
    Authenticate the server against the TLS certificate found in file /home/user/server.crt.
    Fail if the certificate is not present in the indicated location on the client host.
  </dd>

  <dt><code>monetdbs://mdb.example.com/demo?certhash={sha256}fb6720aa009f334c</code>
  <dd>
    Connect to mdb.example.com, secure the connection with TLS.  Do not
    verify the certificate chain but require the hash of the certificate to start with the given
    hexadecimal digits. The hash algorithm is given between braces.
  </dd>

  <dt><code>monetdb:///demo?sock=/var/monetdb/_sock&user=dbuser</code></dt>
  <dd>
    Connect using the given Unix Domain socket in the client's file system. Do not use TCP.
    This syntax departs from the old URL syntax, where it would be
    <code>mapi:monet:///tmp/.s.monetdb.50000?database=demo</code>.
  </dd>

</dl>


## Parameters

The following 24 parameters determine how the connection to MonetDB is
established. Most of them are used as a query parameter in the MonetDB URL,
except for those that are marked as 'core'.
Implementations SHOULD reject URLs in which parameters marked as '(core)' are
passed as query parameters.

Implementations MUST accept all parameters in the table, even if they
do not support the corresponding feature. For example, if the url
`monetdb://localhost/?sock=/path/to/socket is passed to an
implementation that does not support Unix domain sockets, it should
raise an error along the lines of "Unix domain sockets are not
supported", not "invalid parameter: 'sock'".

What happens if an implementation encounters an parameter not in this table
depends on whether or not the name of the parameter contains an underscore. If
the name contains an underscore, the unknown parameter SHOULD be ignored. If it
does not, it SHOULD be rejected. This allows implementations to accept additional
parameters without clashes with official names. It also covers a number of
existing settings specific to monetdb-jdbc.

| Parameter       | Type    | Default     | Remark                                                                                  |
| --------------- | ------- | ----------- | --------------------------------------------------------------------------------------- |
| **tls**         | bool    | false       | (core) secure the connection using TLS                                                  |
| **host**        | string  | absent      | (core) IP number, domain name or one of the special values `localhost` and `localhost.` |
| **port**        | integer | 50000       | (core) TCP port, also used to pick Unix Domain socket path                              |
| **database**    | string  | ""          | (core) name of database to connect to                                                   |
| **tableschema** | string  | absent      | (core) only used for REMOTE TABLE, otherwise unused                                     |
| **table**       | string  | absent      | (core) only used for REMOTE TABLE, otherwise unused                                     |
| **sock**        | path    | absent      | path to Unix Domain socket to connect to                                                |
| **cert**        | path    | absent      | path to TLS certificate to authenticate server with                                     |
| **certhash**    | string  | absent      | hash of server TLS certificate must start with these hex digits; overrides cert         |
| **clientkey**   | path    | absent      | path to TLS key (+certs) to authenticate with as client                                 |
| **clientcert**  | path    | absent      | path to TLS certs for 'clientkey', if not included there                                |
| **user**        | string  | unspecified | user name to authenticate as                                                            |
| **password**    | string  | unspecified | password to authenticate with                                                           |
| **language**    | string  | "sql"       | for example, "sql", "mal", "msql", "profiler"                                           |
| **autocommit**  | bool    | absent      | initial value of autocommit                                                             |
| **schema**      | string  | absent      | initial schema                                                                          |
| **timezone**    | integer | absent      | client time zone as minutes east of UTC                                                 |
| **binary**      | string  | "on"        | whether to use binary result set format (number or bool)                                |
| **replysize**   | integer | absent      | rows beyond this limit are retrieved on demand, <1 means unlimited                      |
| **fetchsize**   | integer | --          | alias for replysize, specific to jdbc                                                   |
| **maxprefetch** | integer | unspecified | specific to pymonetdb                                                                   |
| **hash**        | string  | unspecified | specific to jdbc                                                                        |
| **debug**       | bool    | unspecified | specific to jdbc                                                                        |
| **logfile**     | string  | unspecified | specific to jdbc                                                                        |

In this table, the default 'absent' means no value is given. In Python that
would be a `None`, in C and Java a `null`, etc. 'Unspecified' means that
implementations pick their own default.

In parameter **host**, value 'localhost' is equivalent to the value being
absent. This means the implementation should try both Unix domain sockets and
TCP sockets. The special string 'localhost.', with a trailing period, means
localhost TCP-only.

The default for the parameters **user** and **password** is left unspecified
because historically, different implementations have used different defaults.

Parameter **fetchsize** is a true alias for **replysize**. This means that
whenever an implementation encounters **fetchsize**, it should treat it as
**replysize**. See also [Section Combining multiple
sources](#combining-multiple-sources).

TODO BEFORE 1.0: Decide whether to further define the meaning of **replysize**,
**fetchsize** and **maxprefetch**.


## Combining multiple sources

Often multiple sources of information must be combined to arrive at the final
set of connection parameters. For example,

* mclient first parses its command line parameters. Then, if the
  database parameter turns out to be a URL, it parses the URL and
  potentially overrides the information it got from the command line
  parameters. Also, at some point it tries to apply settings from a
  file `.monetdb`.

* In the JDBC API the driver receives a URL in combination with a
  [Properties] object. Again, the information in the URL overrides
  that from the Properties object.

  [Properties]: https://docs.oracle.com/javase/8/docs/api/java/util/Properties.html

* When connecting to monetdbd, monetdbd can redirect the client to a
  new URL.  This, too is a situation in which sources are combined
  because the new URL contains a new scheme, host name, port, database
  or socket, but other parameters should be retained from the existing
  connection.

When combining connection parameters from multiple sources, the following rules
apply:

1. Every source contributes a subset of the parameters.  If there is overlap,
   later sources take precedence over earlier sources.

2. The defaults listed in [Section Parameters](#parameters) are considered the
   earliest source.

2. A source that sets **user** MUST set **password**. If no sensible value is
   available, **password** SHOULD be set to the empty string, even if that is
   not the default value for this implementation.

3. As described in [Section](#parameters), **fetchsize** MUST be treated as an
   alias of **replysize**. If within one source, the parameters have an order,
   such as in a URL, the last occurrence (of either) wins. If there is no order,
   such as in a Java Properties object, **replysize** takes precedence over
   **fetchsize**.

4. If the source is a URL, all of **tls**, **host**, **port** and **database** MUST
   be set. If not specified in the URL, for example, `monetdb:///`, the default
   values must be used. This does not necessarily apply to other sources.

TODO BEFORE 1.0: It actually makes more sense to me to apply the information
from the URL first and then override it with the auxiliary information such as
command line options and properties. But that would be an incompatible change to
mclient and JDBC. Discuss with Sjoerd.


## URL Syntax

The URL is parsed according to the rules in [RFC 3986][rfc3986]. The general form is:

<pre>
 monetdb://[host[:port]]/[database[/tableschema[/table]]][?param1=value1[&param2=value2...]]
monetdbs://[host[:port]]/[database[/tableschema[/table]]][?param1=value1[&param2=value2...]]
monetdbe:/to/be/determined
</pre>

The `monetdb:` scheme component maps to connection parameter `tls=off`.
The `monetdbs:` scheme component maps to `tls=on`.
The `monetdbe:` scheme is not covered by this version of the specification.

The host, port, database, tableschema and table fields map directly to the
corresponding connection parameters. The latter three MUST be percent-decoded
first ([RFC3986 Section 2.1][rfc3986percent]).

The port number must be positive and at most 65535.

The query parameters, that is, **param1**, **value1**, **param2**, **value2**,
etc., MUST also be percent-decoded, separately. After decoding they MUST be used
as additional connection parameters as described above. However, the parameters
marked '(core)' in the table above, such as **host**, **port** and **database**,
cannot be set in this way. If these occur as query parameters the implementation
MUST raise an error.

If a parameter occurs multiple times, the last occurrence wins.

Boolean values can be written as on/off, yes/no or true/false.

Implementations MUST understand the square brackets IPv6-literal syntax
([RFC3986 Section 3.2.2][rfc3986host]). For example,
`monetdb://[2001::2a]:12345/demo` MUST be parsed even if they underlying
platform does not support IPv6.

According to the RFC, IPv4 and IPv6 addresses are not percent-encoded but
regular host names are. However, implementations MAY choose to ignore this
distinction and percent-decode everything if that is more convenient.
Similarly, when presented with a percent-encoded port implementations MAY choose
to percent-decode it and see if this results in a suitable decimal number even
though the RFC says that the port is not supposed to be percent encoded.

TODO BEFORE 1.0: when we have a number of working implementations, say
pymonetdb, jdbc and libmapi, revisit the allowances above to see if they are
really needed. If all the platforms URL parsers do a great job it may not be
needed.

[rfc3986percent]: https://datatracker.ietf.org/doc/html/rfc3986#section-2.1
[rfc3986host]: https://datatracker.ietf.org/doc/html/rfc3986#section-3.2.2


## Interpreting the parameters

Before trying to connect, an implementation MUST verify that the
parameters satisfy the following constraints.

1. The parameters have the types listed in the table in [Section
   Parameters](#parameters).

2. If **sock** and **host** are both present, **host** must be equal
   to `localhost`. *(Unix domain sockets do not make sense anywhere
   except on the local host, and URL syntax does not allow a port number
   without some form of host identifier, hence, `localhost`.)*

3. The string parameter **binary** must either parse as a boolean or as a
   non-negative integer.

4. If **sock** is present, **tls** must be 'off'.

5. Parameter **database** must consist only of upper- and lowercase letters,
   digits, dashes and underscores. It must not start with a dash.

TODO BEFORE 1.0: figure out exactly where in the source
the name constraints on databases are defined.
I only found something in merovingian.

Based on the given parameters, the implementation should compute a number of
'virtual' parameters.

* Virtual parameter **connect_unix** (a path) indicates whether to try to
  connect over a Unix domain socket, and if so, where. Take the first
  alternative that applies:

  1. if **sock** is present, **connect_unix** has that value.
  2. otherwise, if **tls** is True, **connect_unix** is absent.
  3. otherwise, if **host** is absent or exactly equal to 'localhost',
     **connect_unix** is <code>/tmp/.s.monetdb.<b>port</b></code>.
  4. otherwise, **connect_unix** is absent.

* Virtual parameter **connect_tcp** (a host name or ip number) indicates whether
  to try to connect over TCP, and if so, to which host. Note that the host can
  expand to more than one IP number. In that case, the implementation must try
  them all.

  1. if **sock** is present, **connect_tcp** is absent.
  2. otherwise, if **host** is absent or exactly equal to 'localhost.' (with a trailing
     period), **connect_tcp** is 'localhost'.
  3. otherwise, **connect_tcp** is equal to **host**.

* Virtual parameter **connect_tls_verify** indicates how to verify the TLS
  certificate of the server.

  1. if **tls** is 'off', **connect_tls_verify** is absent.
  2. otherwise, if **certhash** is present, **connect_tls_verify** is 'hash'.
  3. otherwise, if **cert** is present, **connect_tls_verify** is 'cert'.
  4. otherwise, **connect_tls_verify** is 'system'.

* Virtual parameter **connect_binary** (an integer) is the interpretation of the
  string parameter **binary**

  1. if **binary** parses as an integer, **connect_binary** is that integer
  2. if **binary** parses as the boolean True, **connect_binary** is a suitably high
     number such as 65535.
  3. if **binary** parses as the boolean False, **connect_binary** is 0.


TODO BEFORE 1.0: do we allow TLS parameters such as **cert** and **certhash** when
TLS is off? Reject or ignore?


## Connecting


The procedure for establishing a connection is as follows. If the server sends a
redirect, multiple iterations may be necessary.

1. Validate the parameters as described in the previous section.

2. If Unix domain sockets are supported and  **connect_unix** is present, try to
   establish a connection to socket **connect_unix**. If this succeeds, skip the
   next step. If it fails, remember the error message and continue with the next
   step.

3. If **connect_tcp** is present, try to establish a TCP connection to port
   **port** on that host. Note that the given host name may map to more than one
   IP, and a mix of IPv4 and IPv6 addresses.

4. If an error occurred, abort with this error message. Otherwise, continue with
   the next step.

5. If **tls** is enabled, perform a TLS handshake.

   * If **clientkey** is present, load the key and if given the certificates in
     **clientcert**. Offer them to the server as a client certificate. Abort with
     an error if key and certificates cannot be read.

  * If **connect_tls_verify** is 'hash', verify that the hash of certificate
    offered by the server matches **certhash**. The certificate matches if the
    hexadecimal representation of the hash starts with the hexadecimal digits in
    **certhash**. Note that **certhash** must match the leaf certificate of the
    server, not any certificate higher up in the certificate chain. Abort if the
    certificate does not match.

   * If **connect_tls_verify** is 'cert', load the certificate from file
     **cert**. Abort if that fails. Verify the certificate chain offered by the
     server against this certificate and abort if this fails.

  * Finally, if **connect_tls_verify** is 'system', verify the certificate chain
    against the platforms trusted root certificate store and abort if this
    fails.

5. Perform a MAPI login using **user**, **password**, **database** and
   **language**. Use dummy values for **user** and **password** if the server
   announces itself as 'merovingian' rather than 'mserver'.

   * If the server sends an error message, abort with that message.
   * If sends a MAPI redirect to the exact URL `mapi:merovingian://proxy`,
     restart the current step.
   * If it sends a redirect to any other URL, parse the URL as an additional
     source in the sense of [Section Combining multiple
     sources](#combining-multiple-sources) and start over at Step 1.

6. Further configure the connection using **autocommit**, **timezone**, **replysize**
   and **schema**. If binary is supported, combine **connect_binary** with the binary
   level advertised by the server and remember the minimum for later use.


## Parsing classic mapi:monetdb: URLs

Every implementation also needs to be able to deal with the classic
`mapi:monetdb:` URLs because they may occur in monetdbd's redirect messages
while logging in. They can also occur in the output of `monetdb status`.

Note: pymonetdb supports an extended `mapi:monetdb:` syntax which includes user
name, password, and a few more options:

    mapi:monetdb://user:password@host:port/database

However, this extension is specific to pymonetdb and other implementations
SHOULD NOT support it.

This is how the classic URLs should be interpreted:

* The URL starts with `mapi:monetdb://`. There is no `mapi:monetdbs:` variant and
  there never will be. The prefix `mapi:merovingian:` is used internally in the
  MAPI protocol but it is never visible to users and it is not governed by this
  specification.

* No percent decoding is performed anywhere, at all.

* Host name and port are passed in the usual way after the double slashes.

* The `localhost.` syntax does not exist. Since it's not a valid domain
  name anyway, implementations SHOULD just reject it.

* If a host name is given, the path component of the URL is interpreted as the
  database name.

* If no host name is given, that is, if the URL starts with `mapi:monetdb:///`,
  the path component is interpreted as a file system path to a Unix Domain socket
  to connect to.

* Only two query parameters are recognized: `language` and `database`.
  The latter is only only allowed with Unix Domain sockets, that is,
  triple-slash URLs.

* All other parameters MUST be ignored.
