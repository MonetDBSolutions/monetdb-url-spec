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

TODO BEFORE 0.9: field / property / attribute be consistent


## Examples

<dl>

  <dt><code>monetdb:///demo</code></dt>
  <dd>
    Scan /tmp for Unix domain sockets named <code>/tmp/.s.monetdb.<b>&lt;integer 1-65535&gt;</b></code>.
    Connect to them one by one and try to log in to database 'demo'.
    Return the first connection where this succeeds.
    Try TCP localhost port 50000 if none succeed.
    Do not secure the connection using TLS.
    This is the behavior of libmapi if neither host nor port is given but database is.
    User name and password are not given in the URL and must be provided externally.
    On Windows, no equivalent for /tmp has been defined so only TCP is tried.
  </dd>

  <dt><code>monetdb://localhost/demo</code></dt>
  <dd>
    Identical to the above. This is because many URL parsers do not allow a port
    number if no host is given, so 'localhost' is treated as 'no host'.
  </dd>

  <dt><code>monetdb://localhost./demo</code></dt>
  <dd>
    Try to make a TCP connection to localhost, port 50000.
    So, if you really mean 'localhost', write 'localhost.' with a trailing period.
  </dd>

  <dt><code>monetdb://localhost.:12345/demo</code></dt>
  <dd>
    Same as above, but port 12345.
  </dd>

  <dt><code>monetdb://localhost:12345/demo</code></dt>
  <dd>
    This is 'localhost' without the trailing period.
    First, try to connect to Unix domain socket /tmp/.s.monetdb.12345.
    If this does not succeed, fall back to TCP localhost port 12345.
    If a connection was established, try to log in etc.
    This is more Unix domain magic to stay compatible with libmapi.
    Note that as opposed to <code>monetdb:///demo</code>,
    a login failure on the Unix socket does not cause an attempt on the
    TCP socket.
  </dd>

  <dt><code>monetdb:///demo?user=monetdb&password=monetdb</code></dt>
  <dd>
    User name and password are part of the URL.
  </dd>

  <dt><code>monetdb://mdb.example.com:12345/demo</code></dt>
  <dd>
    Try to connect to database 'demo' on TCP port 12345 on mdb.example.com.
  </dd>

  <dt><code>monetdb://192.168.13.4:12345/demo</code></dt>
  <dd>
    Try to connect to database 'demo' on TCP port 12345 on the given IPv4 address.
  </dd>

  <dt><code>monetdb://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:12345/demo</code></dt>
  <dd>
    Try to connect to database 'demo' on TCP port 12345 on the given IPv6 address.
  </dd>

  <dt><code>monetdb://localhost/</code></dt>
  <dd>
    Try to connect to an unspecified database on <code>/tmp/.s.monetdb.50000</code>
    or if that fails, TCP port 50000 on localhost. Because no database name is given,
    this will only work when connecting to a raw mserver5.
  </dd>

  <dt><code>monetdbs://mdb.example.com/demo</code></dt>
  <dd>
    (Note the 's' in <code>monetdbs</code>.)
    Similar to <code>monetdb://mdb.example.com/demo</code>, but secure the connection
    using TLS. Use root certificates from the system trusted certificate store to
    authenticate the server.
  </dd>

  <dt><code>monetdbs:///demo</code></dt>
  <dd>
    Where <code>monetdb:///demo</code> caused a complicated scanning procedure,
    <code>monetdbs:///demo</code> is always a simple TCP connection.
    This is because TLS is not compatible with the way MonetDB uses Unix domain
    sockets.
  </dd>

  <dt><code>monetdbs://mdb.example.com/demo?cert=/home/user/server.crt</code></dt>
  <dd>
    Connect to mdb.example.com.
    Authenticate the server against the TLS certificate found in file /home/user/server.crt.
    Fail if the certificate is not present in the indicated location on the client host.
  </dd>

  <dt><code>monetdbs://mdb.example.com/demo?certhash={sha256}fb:67:20:aa:00:9f:33:4c</code></dt>
  <dd>
    Connect to mdb.example.com, secure the connection with TLS.  Do not
    verify the certificate chain but require the hash of the certificate to start with the given hexadecimal digits.
    The hash algorithm (always SHA-256) is given between braces.
    The colons are optional.
  </dd>

  <dt><code>monetdb:///demo?sock=/var/monetdb/_sock&user=dbuser</code></dt>
  <dd>
    Connect using the given Unix domain socket in the client's file system. Do not use TCP.
    Note: this syntax departs from the old URL syntax, where it would be
    <code>mapi:monet:///var/monetdb/_sock?database=demo</code>.
  </dd>


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
| **host**        | string  | ""          | (core) IP number, domain name or one of the special values `localhost` and `localhost.` |
| **port**        | integer | -1          | (core) Port to connect to, 1..65535 or -1 for 'not set'                                 |
| **database**    | string  | ""          | (core) name of database to connect to                                                   |
| **tableschema** | string  | ""          | (core) only used for REMOTE TABLE, otherwise unused                                     |
| **table**       | string  | ""          | (core) only used for REMOTE TABLE, otherwise unused                                     |
| **sock**        | path    | ""          | path to Unix Domain socket to connect to                                                |
| **cert**        | path    | ""          | path to TLS certificate to authenticate server with                                     |
| **certhash**    | string  | ""          | hash of server TLS certificate must start with these hex digits; overrides cert         |
| **clientkey**   | path    | ""          | path to TLS key (+certs) to authenticate with as client                                 |
| **clientcert**  | path    | ""          | path to TLS certs for 'clientkey', if not included there                                |
| **user**        | string  | unspecified | user name to authenticate as                                                            |
| **password**    | string  | unspecified | password to authenticate with                                                           |
| **language**    | string  | "sql"       | for example, "sql", "mal", "msql", "profiler"                                           |
| **autocommit**  | bool    | unspecified | initial value of autocommit                                                             |
| **schema**      | string  | ""          | initial schema                                                                          |
| **timezone**    | integer | unspecified | client time zone as minutes east of UTC                                                 |
| **binary**      | string  | "on"        | whether to use binary result set format (number or bool)                                |
| **replysize**   | integer | unspecified | rows beyond this limit are retrieved on demand, <1 means unlimited                      |
| **fetchsize**   | integer | --          | alias for replysize, specific to jdbc                                                   |
| **maxprefetch** | integer | unspecified | specific to pymonetdb                                                                   |
| **hash**        | string  | unspecified | specific to jdbc                                                                        |
| **debug**       | bool    | unspecified | specific to jdbc                                                                        |
| **logfile**     | string  | unspecified | specific to jdbc                                                                        |

The rules for interpreting parameter **host** are complicated and are
described in the next sections.

Some default values have been left unspecified because they vary between the
existing implementations. The most problematic of these are **user** and
**password**, which sometimes default to monetdb/monetdb and sometimes to
no value.

Parameter **fetchsize** is a true alias for **replysize**. This means that
whenever an implementation encounters **fetchsize**, it should treat it as
**replysize**.
See also [Section Combining multiple sources](#combining-multiple-sources).


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
  new URL.  This too is a situation in which sources are combined
  because the new URL contains a new scheme, host name, port, database
  or socket, but other parameters should be retained from the existing
  connection.

When combining connection parameters from multiple sources, the following rules
apply:

1. Every source contributes a subset of the parameters.  If there is overlap,
   later sources take precedence over earlier sources.

2. The defaults listed in [Section Parameters](#parameters) are considered the
   earliest source.

3. A source that changes **user** MUST change **password**. If no sensible value is
   available, **password** SHOULD be set to the empty string, even if that is
   not the default value for this implementation.

4. As described in [Section](#parameters), **fetchsize** MUST be treated as an
   alias of **replysize**. If within one source, the parameters have an order,
   such as in a URL, the last occurrence (of either) SHOULD win. If there is no order,
   such as in a Java Properties object, **replysize** SHOULD take precedence over
   **fetchsize**.

5. If the source is a URL, all of **tls**, **host**, **port** and **database** MUST
   be set. If not specified in the URL, as for example in `monetdb:///`, they
   should be reset to their default.
   This does not necessarily apply to other sources.


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

The host, port, database, tableschema and table fields map to the corresponding
connection parameters. The latter three MUST be percent-decoded first ([RFC3986
Section 2.1][rfc3986percent]). Fields that are not present are mapped to empty
strings or in the case of port, to  -1.

If the host field is equal to 'localhost' (without period), the **host**
parameter MUST be cleared. If the host field is equal to 'localhost.' (with
period), the **host** parameter MUST be set to 'localhost' (without period).

The latter rule is necessary because some URL parsing libraries do not accept a
port number without a host name. This way, 'localhost' can stand in for 'host
not set' and 'localhost.' can stand in for 'localhost'. Note that this is only
in the URL.

The port number must be positive and at most 65535.

Boolean values can be written as on/off, yes/no or true/false. Case does not matter.

Integer values are written as decimal integers. In particular, hexadecimals are
not supported and a leading zero does not cause the value to be interpreted as
an octal number.

The query parameters, that is, **param1**, **value1**, **param2**, **value2**,
etc., MUST also be percent-decoded, separately. After decoding they MUST be used
as additional connection parameters as described above. However, the parameters
marked '(core)' in the table above, such as **host**, **port** and **database**,
cannot be set in this way. If these occur as query parameters the implementation
MUST raise an error.

If a parameter occurs multiple times, the last occurrence wins.

Implementations MUST understand the square brackets IPv6-literal syntax
([RFC3986 Section 3.2.2][rfc3986host]). For example,
`monetdb://[2001::2a]:12345/demo` MUST be parsed even if they underlying
platform does not support IPv6.

According to RFC 3986, IPv4 and IPv6 addresses are not supposed to be
percent-encoded but regular host names are. In MonetDB URLs however,
if an implementation encounters a percent-encoded IP number, it MAY
decode and accept it, but it MAY also reject it, whatever is more convenient.

[rfc3986percent]: https://datatracker.ietf.org/doc/html/rfc3986#section-2.1
[rfc3986host]: https://datatracker.ietf.org/doc/html/rfc3986#section-3.2.2


## Interpreting the parameters

Before trying to connect, an implementation MUST verify that the
parameters satisfy the following constraints.

1. The parameters have the types listed in the table in [Section
   Parameters](#parameters).

2. At least one of **sock** and **host** must be empty.

3. The string parameter **binary** must either parse as a boolean or as a
   non-negative integer.

4. If **sock** is not empty, **tls** must be 'off'.

5. If **certhash** is not empty, it must be of the form `{sha256}hexdigits`
   where hexdigits is a non-empty sequence of 0-9, a-f, A-F and colons.

6. If **tls** is 'off', **cert** and **certhash** must be 'off' as well.

7. Parameters **database**, **tableschema** and **table** must consist only of
   upper- and lowercase letters, digits, dashes and underscores. They must not
   start with a dash.

8. Parameter **port** must be -1 or in the range 1-65535.

9. If **clientcert** is set, **clientkey** must also be set.

TODO BEFORE 0.9: figure out exactly where in the source
the name constraints on databases are defined.
I only found something in merovingian.

Based on the given parameters, the implementation should compute a number of
'virtual' parameters.

* Virtual parameter **connect_scan** is true if and only if **database** is not
  empty, **sock**, **host** and **port** are all empty/-1, and **tls** is 'off'.

* Virtual parameter **connect_unix** (a path) indicates whether to try to
  connect over a Unix domain socket, and if so, where. Take the first
  alternative that applies:

  1. if **sock** is not empty, **connect_unix** has that value.
  2. otherwise, if **tls** is True, **connect_unix** is empty.
  3. otherwise, if **host** is empty, **connect_unix** is derived from the port
     number as follows: <code>/tmp/.s.monetdb.<b>connect_port</b></code>.
  4. otherwise, **connect_unix** is empty.

* Virtual parameter **connect_tcp** (a host name or ip number) indicates whether
  to try to connect over TCP, and if so, to which host. Note that the host can
  expand to more than one IP number. In that case, the implementation must try
  them all.

  1. if **sock** is not empty, **connect_tcp** is empty.
  2. otherwise, if **host** is empty, **connect_tcp** is 'localhost'.
  3. otherwise, **connect_tcp** is equal to **host**.

* Virtual parameter **connect_port** indicates the port to try to connect to over TCP.

  1. if **port** is -1, **connect_port** is 50000.
  2. otherwise, **connect_port** has the value of **port**.

* Virtual parameter **connect_tls_verify** indicates how to verify the TLS
  certificate of the server.

  1. if **tls** is 'off', **connect_tls_verify** is empty.
  2. otherwise, if **certhash** is not empty, **connect_tls_verify** is 'hash'.
  3. otherwise, if **cert** is not empty, **connect_tls_verify** is 'cert'.
  4. otherwise, **connect_tls_verify** is 'system'.

* Virtual parameter **connect_certhash_digits** gives the hexdigits to
  compare to, with colons stripped.

  1. if **tls** is 'off' or **certhash** is empty, **connect_certhash_digits** is
     empty.
  2. otherwise, certhash is the value of **certhash** in lowercase, with the
     hash name prefix and all colons stripped.

* Virtual parameter **connect_binary** (an integer) is the interpretation of the
  string parameter **binary**

  1. if **binary** parses as an integer, **connect_binary** is that integer
  2. if **binary** parses as the boolean True, **connect_binary** is a suitably high
     number such as 65535.
  3. if **binary** parses as the boolean False, **connect_binary** is 0.

* Virtual parameter **connect_clientkey** is the path of a file holding the
  private key used to authenticate to the server (MTLS).

  1. It is always equal to **clientkey**.

* Virtual parameter **connect_clientcert** is the path of a file holding the
  certificates to offer together with key **connect_clientkey** when
  authenticating to the server (MTLS).

  1. If **clientcert** is not empty, use that.
  2. Otherwise, use the value of **connect_clientkey**.


## Connecting

The procedure for establishing a connection is given below. In many cases, multiple
iterations are necessary.

In combination with the way the virtual parameters are computed in the previous
section, this procedure is intended to be identical to the one described in the
[comment at the start of `mapi_reconnect` in mapi.c][mapi_reconnect]
in the MonetDB source code.

[mapi_reconnect]: https://github.com/MonetDB/MonetDB/blob/d1ac6fe66bf095e17c40a863acc348bdb4aceb93/clients/mapilib/mapi.c#L2264-L2284

1. Validate the parameters and compute virtual parameters as described in the
   previous section.

2. If **connect_scan** is true, follow the procedure described in [Scanning Unix
   domain socket](#scanning-unix-domain-sockets) instead of the procedure in this
   section.

3. If Unix domain sockets are supported and  **connect_unix** is not empty, try to
   establish a connection to socket **connect_unix**. If this succeeds, skip the
   next step. If it fails, remember the error message and continue with the next
   step.

4. If **connect_tcp** is not empty, try to establish a TCP connection to port
   **connect_port** on that host. This may involve more than one attempt because
   a host name often maps to more than one IP address.

5. If an error has occurred, abort with that error message. Otherwise, continue with
   the next step.

6. If **tls** is enabled, perform a TLS handshake.

    * If **connect_clientkey** is not empty, load the private key from that file.
      Same for the certificate chain in **connect_clientcert**.
      Offer them to the server as a client certificate. Abort with
      an error if key or certificates cannot be read.

    * If **connect_tls_verify** is 'hash', compute the SHA-256 digest of the certificate
      offered by the server.
      Convert it to lowercase hex digits and abort if this does not start with
      **connect_certhash_digits**.

      Note: use only the leaf certificate in the certificate chain. The other
      certificates in the chain MUST be ignored. They MUST NOT be compared against
      the hash and they MUST NOT be verified against any root certificate.

    * If **connect_tls_verify** is 'cert', read a certificate from file
      **cert**. Abort if that fails. Verify the certificate chain offered by the
      server against this certificate and abort if this fails.

    * Finally, if **connect_tls_verify** is 'system', verify the certificate chain
      against the platforms trusted root certificate store and abort if this
      fails.

7. Perform a MAPI login using **user**, **password**, **database** and
   **language**. Use dummy values for **user** and **password** if the server
   announces itself as 'merovingian' rather than 'mserver'.

    * If the server sends an error message, abort with that message.

    * If sends a MAPI redirect to the exact URL `mapi:merovingian://proxy`,
      restart the current step.

    * If it sends a redirect to any other URL, parse the URL as an additional
      source in the sense of [Section Combining multiple
      sources](#combining-multiple-sources) and start over at Step 1.

8. Further configure the connection using **autocommit**, **timezone**, **replysize**
   and **schema**. If binary is supported, combine **connect_binary** with the binary
   level advertised by the server and remember the minimum for later use.


### Scanning Unix domain sockets

When connecting to `monetdb:///dbname`, that is, with **database** set, **host**
and **port** unset and **tls** off, special behavior kicks in, very succinctly
described in [the comment in mapi.c][mapi_reconnect]:

1. The implementation scans /tmp for sockets with names of the form
   <code>/tmp/.s.monetdb.<b>port</b></code>, with <code><b>port</b></code> a
   valid port number. It also notes the owning uid of those sockets.

2. The implementation orders the socket found in such a way sockets that are
   owned by the current process come before sockets that are not.

3. Then, for each socket in turn, the implementation tries to connect using the
   procedure listed in [Section Connecting](#connecting), with **sock** updated
   to the socket in question.The first connection which fully succeeds,
   including mapi handshake, is returned.

4. If no connection attempt succeeded, the implementation now tries
   to connect with **host** set to 'localhost'.


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
