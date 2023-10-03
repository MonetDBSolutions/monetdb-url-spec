# Tests

This document contains a large number of test cases.
They are embedded in the Markdown source, in <code>```test</code>
. . .</code>```</code> blocks.


The tests are written in a mini language with the following
keywords:

* `PARSE url`: parse the URL, this should succeed. The validity checks need
  not be satisfied.

* `ACCEPT url`: parse the URL, this should succeed. The validity checks
  should pass.

* `REJECT url`: parse the URL, it should be rejected either in the parsing stage
  or by the validity checks.

* `SET key=value`: modify a parameter, can occur before or after parsing the URL.
  Used to model command line parameters, Java Properties objects, etc.

* `EXPECT key=value`: verify that the given parameter now has the given
  value. Fail the test case if the value is different.

At the start of each block the parameters are reset to their default values.

The EXPECT clause can verify all parameters listen in the Parameters section of
the spec, all 'virtual parameters' and also the special case `valid` which is a
boolean indicating whether all validity rules in section 'Interpreting the
parameters' hold.

Note: an `EXPECT` of the virtual parameters implies `EXPECT valid=true`,
as do `PARSE` and `REJECT`. In the case of `PARSE`, `valid` must be true.
In the case of `REJECT`, if the URL is syntactically correct but `valid` is false,
this is considered a succesful rejection of the URL.

TODO before 1.0 does the above explanation make sense?


## Tests from the examples

```test
ACCEPT monetdb://localhost:50000/demo?user=monetdb&password=monetdb
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=demo
EXPECT user=monetdb
EXPECT password=monetdb
```

```test
ACCEPT monetdb:///demo
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=demo
```

```test
ACCEPT monetdb://localhost./demo
EXPECT connect_unix=
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=demo
```

```test
ACCEPT monetdb://localhost:50000/demo?user=monetdb&password=monetdb
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=demo
EXPECT user=monetdb
EXPECT password=monetdb
```

```test
ACCEPT monetdb://mdb.example.com:12345/demo
EXPECT connect_unix=
EXPECT connect_tcp=mdb.example.com
EXPECT port=12345
EXPECT tls=off
EXPECT database=demo
```

```test
ACCEPT monetdb://192.168.13.4:12345/demo
EXPECT connect_unix=
EXPECT connect_tcp=192.168.13.4
EXPECT port=12345
EXPECT tls=off
EXPECT database=demo
```

```test
ACCEPT monetdb://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:12345/demo
EXPECT connect_unix=
EXPECT connect_tcp=2001:0db8:85a3:0000:0000:8a2e:0370:7334
EXPECT port=12345
EXPECT tls=off
EXPECT database=demo
```

```test
ACCEPT monetdb://localhost/
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=
```

```test
ACCEPT monetdb://localhost
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=
```

```test
ACCEPT monetdbs://mdb.example.com/demo
EXPECT connect_unix=
EXPECT connect_tcp=mdb.example.com
EXPECT port=50000
EXPECT tls=on
EXPECT connect_tls_verify=system
EXPECT database=demo
```

```test
ACCEPT monetdbs://mdb.example.com/demo?cert=/home/user/server.crt
EXPECT connect_unix=
EXPECT connect_tcp=mdb.example.com
EXPECT port=50000
EXPECT tls=on
EXPECT connect_tls_verify=cert
EXPECT cert=/home/user/server.crt
EXPECT database=demo
```

```test
ACCEPT monetdbs://mdb.example.com/demo?certhash={sha256}fb:67:20:aa:00:9f:33:4c
EXPECT connect_unix=
EXPECT connect_tcp=mdb.example.com
EXPECT port=50000
EXPECT tls=on
EXPECT connect_tls_verify=hash
EXPECT certhash={sha256}fb:67:20:aa:00:9f:33:4c
EXPECT connect_certhash_algo=sha256
EXPECT connect_certhash_digits=fb6720aa009f334c
EXPECT database=demo
```

```test
ACCEPT monetdbs://mdb.example.com/demo?certhash={sha256}A::B
EXPECT connect_certhash_algo=sha256
EXPECT connect_certhash_digits=ab
```

```test
ACCEPT monetdbs://mdb.example.com/demo?certhash={sHA256}A::B
EXPECT connect_certhash_algo=sha256
EXPECT connect_certhash_digits=ab
```

```test
ACCEPT monetdbs://mdb.example.com/demo?certhash={sHA1}A::B
EXPECT connect_certhash_algo=sha1
EXPECT connect_certhash_digits=ab
```

```test
ACCEPT monetdbs://mdb.example.com/demo?certhash=A::B
EXPECT connect_certhash_algo=sha1
EXPECT connect_certhash_digits=ab
```

```test
REJECT monetdbs://mdb.example.com/demo?certhash=X
REJECT monetdbs://mdb.example.com/demo?certhash={sha1}X
REJECT monetdbs://mdb.example.com/demo?certhash={sha99}X
REJECT monetdbs://mdb.example.com/demo?certhash={X
REJECT monetdbs://mdb.example.com/demo?certhash={banana}abcdef
```


case errors


```test
ACCEPT monetdb:///demo?sock=/var/monetdb/_sock&user=dbuser
EXPECT connect_unix=/var/monetdb/_sock
EXPECT connect_tcp=
EXPECT tls=off
EXPECT database=demo
EXPECT user=dbuser
EXPECT password=
```

## Parameter tests

Tests derived from the parameter section. Test data types and defaults.

### core defaults

```test
EXPECT tls=false
EXPECT host=
EXPECT port=50000
EXPECT database=
EXPECT tableschema=
EXPECT table=
```

### sock

Nowhere supported on Windows, but they should *parse*!

```test
EXPECT sock=
ACCEPT monetdb:///?sock=/tmp/sock
EXPECT sock=/tmp/sock
ACCEPT monetdb:///?sock=C:\TEMP\sock
EXPECT sock=C:\TEMP\sock
```

### cert

```test
EXPECT cert=
ACCEPT monetdbs:///?cert=/tmp/cert.pem
EXPECT cert=/tmp/cert.pem
ACCEPT monetdbs:///?cert=C:\TEMP\cert.pem
EXPECT cert=C:\TEMP\cert.pem
```

### certhash

```test
EXPECT certhash=
ACCEPT monetdbs:///?certhash={sha1}001122ff
ACCEPT monetdbs:///?certhash={sha1}00:11:22:ff
ACCEPT monetdbs:///?certhash={sha256}001122ff
ACCEPT monetdbs:///?certhash=001122ff
REJECT monetdbs:///?certhash=sha1:001122ff
REJECT monetdbs:///?certhash={sha1}}001122gg
```

### clientkey, clientcert

```test
EXPECT clientkey=
ACCEPT monetdbs:///?clientkey=/tmp/clientkey.pem
EXPECT clientkey=/tmp/clientkey.pem
ACCEPT monetdbs:///?clientkey=C:\TEMP\clientkey.pem
EXPECT clientkey=C:\TEMP\clientkey.pem
```
### clientcert

```test
EXPECT clientcert=
ACCEPT monetdbs:///?clientcert=/tmp/clientcert.pem
EXPECT clientcert=/tmp/clientcert.pem
ACCEPT monetdbs:///?clientcert=C:\TEMP\clientcert.pem
EXPECT clientcert=C:\TEMP\clientcert.pem
```

### user, password

Not testing the default because they are (unfortunately)
implementation specific.

```test
ACCEPT monetdb:///?user=monetdb
EXPECT user=monetdb
ACCEPT monetdb:///?user=me&password=?
EXPECT user=me
EXPECT password=?
```

### language

```test
EXPECT language=sql
ACCEPT monetdb:///?language=msql
EXPECT language=msql
ACCEPT monetdb:///?language=sql
EXPECT language=sql
```

### autocommit

```test
ACCEPT monetdb:///?autocommit=true
EXPECT autocommit=true
ACCEPT monetdb:///?autocommit=on
EXPECT autocommit=true
ACCEPT monetdb:///?autocommit=yes
EXPECT autocommit=true
```

```test
ACCEPT monetdb:///?autocommit=false
EXPECT autocommit=false
ACCEPT monetdb:///?autocommit=off
EXPECT autocommit=false
ACCEPT monetdb:///?autocommit=no
EXPECT autocommit=false
```

```test
REJECT monetdb:///?autocommit=
REJECT monetdb:///?autocommit=banana
REJECT monetdb:///?autocommit=0
REJECT monetdb:///?autocommit=1
```

### schema, timezone

Must be accepted, no constraints on content

```test
EXPECT schema=
ACCEPT monetdb:///?schema=foo
EXPECT schema=foo
ACCEPT monetdb:///?schema=
EXPECT schema=
ACCEPT monetdb:///?schema=foo
```

```test
ACCEPT monetdb:///?timezone=0
EXPECT timezone=0
ACCEPT monetdb:///?timezone=120
EXPECT timezone=120
ACCEPT monetdb:///?timezone=-120
EXPECT timezone=-120
REJECT monetdb:///?timezone=banana
```

### replysize and fetchsize

Note we never check `EXPECT fetchsize=`, it doesn't exist.

```test
ACCEPT monetdb:///?replysize=150
EXPECT replysize=150
ACCEPT monetdb:///?fetchsize=150
EXPECT replysize=150
ACCEPT monetdb:///?fetchsize=100&replysize=200
EXPECT replysize=200
ACCEPT monetdb:///?replysize=100&fetchsize=200
EXPECT replysize=200
```

### binary

```test
EXPECT binary=on
EXPECT connect_binary=65535
```

```test
ACCEPT monetdb:///?binary=on
EXPECT connect_binary=65535

ACCEPT monetdb:///?binary=yes
EXPECT connect_binary=65535

ACCEPT monetdb:///?binary=true
EXPECT connect_binary=65535

ACCEPT monetdb:///?binary=yEs
EXPECT connect_binary=65535
```

```test
ACCEPT monetdb:///?binary=off
EXPECT connect_binary=0

ACCEPT monetdb:///?binary=no
EXPECT connect_binary=0

ACCEPT monetdb:///?binary=false
EXPECT connect_binary=0
```

```test
ACCEPT monetdb:///?binary=0
EXPECT connect_binary=0

ACCEPT monetdb:///?binary=5
EXPECT connect_binary=5

ACCEPT monetdb:///?binary=0100
EXPECT connect_binary=100
```

```test
REJECT monetdb:///?binary=
REJECT monetdb:///?binary=-1
REJECT monetdb:///?binary=1.0
REJECT monetdb:///?binary=banana
```

### unknown parameters

```test
REJECT monetdb:///?banana=bla
ACCEPT monetdb:///?ban_ana=bla
ACCEPT monetdb:///?hash=sha1
ACCEPT monetdb:///?debug=true
ACCEPT monetdb:///?logfile=banana
```

Unfortunately we can't easily test that it won't allow us
to SET banana.

```test
SET ban_ana=bla
SET hash=sha1
SET debug=true
SET logfile=banana
```

## Combining sources

The defaults have been tested in the previous section.

Rule: If there is overlap, later sources take precedence.

```test
SET schema=a
ACCEPT monetdb:///db1?schema=b
EXPECT schema=b
EXPECT database=db1
EXPECT tls=off
ACCEPT monetdbs:///db2?schema=c
EXPECT tls=on
EXPECT database=db2
EXPECT schema=c
```

Rule: a source that sets user must set password or clear.

```skiptest
ACCEPT monetdb:///?user=foo
EXPECT user=foo
EXPECT password=
SET password=banana
EXPECT user=foo
EXPECT password=banana
SET user=bar
EXPECT password=
```

Rule: fetchsize is an alias for replysize, last occurrence counts

```test
SET replysize=200
SET fetchsize=300
EXPECT replysize=300
ACCEPT monetdb:///?fetchsize=400
EXPECT replysize=400
ACCEPT monetdb:///?replysize=500&fetchsize=600
EXPECT replysize=600
```

Rule: parsing a URL sets all of tls, host, port and database
even if left out of the URL

```test
SET tls=on
SET host=banana
SET port=12345
SET database=foo
SET timezone=120
ACCEPT monetdb:///
EXPECT tls=off
EXPECT host=
EXPECT port=50000
EXPECT database=
```

```test
SET tls=on
SET host=banana
SET port=12345
SET database=foo
SET timezone=120
ACCEPT monetdb://dbhost/dbdb
EXPECT tls=off
EXPECT host=dbhost
EXPECT port=50000
EXPECT database=dbdb
```

Careful around passwords

```test
SET user=alan
SET password=turing
ACCEPT monetdbs:///
EXPECT user=alan
EXPECT password=turing
```

```test
SET user=alan
SET password=turing
ACCEPT monetdbs:///?user=mathison
EXPECT user=mathison
EXPECT password=
```

The rule is, "if **user** set", not "if **user** is changed".

```test
SET user=alan
SET password=turing
ACCEPT monetdbs:///?user=alan
EXPECT user=alan
EXPECT password=
```

## URL syntax

General form

```test
ACCEPT monetdb://host:12345/db1/schema2/table3?user=mr&password=bean
EXPECT tls=off
EXPECT host=host
EXPECT port=12345
EXPECT database=db1
EXPECT tableschema=schema2
EXPECT table=table3
EXPECT user=mr
EXPECT password=bean
```

Also, TLS and percent-escapes

```test
ACCEPT monetdbs://h%6Fst:12345/db%31/schema%32/table%33?user=%6Dr&p%61ssword=bean
EXPECT tls=on
EXPECT host=host
EXPECT port=12345
EXPECT database=db1
EXPECT tableschema=schema2
EXPECT table=table3
EXPECT user=mr
EXPECT password=bean
```

Port number

```test
REJECT monetdb://banana:0/
REJECT monetdb://banana:-1/
REJECT monetdb://banana:65536/
REJECT monetdb://banana:100000/
```

Trailing slash can be left off

```test
ACCEPT monetdb://host?user=claude&password=m%26ms
EXPECT host=host
EXPECT user=claude
EXPECT password=m&ms
```

Error to set tls, host, port, database, tableschema and table as query parameters.

```test
REJECT monetdb://foo:1/bar?tls=off
REJECT monetdb://foo:1/bar?host=localhost
REJECT monetdb://foo:1/bar?port=12345
REJECT monetdb://foo:1/bar?database=allmydata
REJECT monetdb://foo:1/bar?tableschema=banana
REJECT monetdb://foo:1/bar?table=tabularity
```

Last wins, already tested elsewhere but for completeness

```test
ACCEPT monetdbs:///?timezone=10&timezone=20
EXPECT timezone=20
```

Interesting case: setting user must clear the password but does
that also happen with repetitions within a URL?
Not sure. For the time being, no. This makes it easier for
situations where for example the query parameters come in
alphabetical order

```test
ACCEPT monetdb:///?user=foo&password=banana&user=bar
EXPECT user=bar
EXPECT password=banana
```

Similar but even simpler: user comes after password but does not
clear it.

```test
ACCEPT monetdb:///?password=pw&user=foo
EXPECT user=foo
EXPECT password=pw
```

Ways of writing booleans and the binary property have already been tested above.

Ip numbers:

```test
ACCEPT monetdb://192.168.1.1:12345/foo
EXPECT connect_unix=
EXPECT connect_tcp=192.168.1.1
EXPECT database=foo
```

```test
ACCEPT monetdb://[::1]:12345/foo
EXPECT connect_unix=
EXPECT connect_tcp=::1
EXPECT database=foo
```

Bad percent escapes:

```test
REJECT monetdb:///m%xxbad
```


## Interpreting

Testing the validity constraints.
They apply both when parsing a URL and with ad-hoc settings.

The type constraints have already been tested above.

The following tests check the interaction between **tls**, **host** and **sock**.

```test
ACCEPT monetdb:///
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
```

```test
ACCEPT monetdb:///?sock=/a/path
EXPECT connect_unix=/a/path
EXPECT connect_tcp=
```

```test
ACCEPT monetdb://localhost/
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
```

```test
ACCEPT monetdb://localhost/?sock=/a/path
EXPECT connect_unix=/a/path
EXPECT connect_tcp=
```

```test
ACCEPT monetdb://localhost./
EXPECT connect_unix=
EXPECT connect_tcp=localhost
```

```test
REJECT monetdb://localhost./?sock=/a/path
```

```test
ACCEPT monetdb://not.localhost/
EXPECT connect_unix=
EXPECT connect_tcp=not.localhost
```

```test
REJECT monetdb://not.localhost/?sock=/a/path
```

```test
ACCEPT monetdbs:///
EXPECT connect_unix=
EXPECT connect_tcp=localhost
```

```test
REJECT monetdbs:///?sock=/a/path
```

```test
ACCEPT monetdbs://localhost/
EXPECT connect_unix=
EXPECT connect_tcp=localhost
```

```test
REJECT monetdbs://localhost/?sock=/a/path
```

```test
ACCEPT monetdbs://localhost./
EXPECT connect_unix=
EXPECT connect_tcp=localhost
```

```test
REJECT monetdbs://localhost./?sock=/a/path
```

```test
ACCEPT monetdbs://not.localhost/
EXPECT connect_unix=
EXPECT connect_tcp=not.localhost
```

```test
REJECT monetdbs://not.localhost/?sock=/a/path
```

```test
SET database=
EXPECT valid=yes
SET database=banana
EXPECT valid=yes
SET database=UPPERCASE
EXPECT valid=yes
SET database=_under_score_
EXPECT valid=yes
SET database=with-dashes
EXPECT valid=yes
```

```test
SET database=with/slash
EXPECT valid=no
SET database=-flag
EXPECT valid=no
SET database=with space
EXPECT valid=no
SET database=with.period
EXPECT valid=no
SET database=with%percent
EXPECT valid=no
SET database=with!exclamation
EXPECT valid=no
SET database=with?questionmark
EXPECT valid=no
```

```test
SET tableschema=
EXPECT valid=yes
SET tableschema=banana
EXPECT valid=yes
SET tableschema=UPPERCASE
EXPECT valid=yes
SET tableschema=_under_score_
EXPECT valid=yes
SET tableschema=with-dashes
EXPECT valid=yes
```

```test
SET tableschema=with/slash
EXPECT valid=no
SET tableschema=-flag
EXPECT valid=no
SET tableschema=with space
EXPECT valid=no
SET tableschema=with.period
EXPECT valid=no
SET tableschema=with%percent
EXPECT valid=no
SET tableschema=with!exclamation
EXPECT valid=no
SET tableschema=with?questionmark
EXPECT valid=no
```

```test
SET table=
EXPECT valid=yes
SET table=banana
EXPECT valid=yes
SET table=UPPERCASE
EXPECT valid=yes
SET table=_under_score_
EXPECT valid=yes
SET table=with-dashes
EXPECT valid=yes
```

```test
SET table=with/slash
EXPECT valid=no
SET table=-flag
EXPECT valid=no
SET table=with space
EXPECT valid=no
SET table=with.period
EXPECT valid=no
SET table=with%percent
EXPECT valid=no
SET table=with!exclamation
EXPECT valid=no
SET table=with?questionmark
EXPECT valid=no
```


# Legacy URL's

```test
REJECT mapi:
REJECT mapi:monetdb
REJECT mapi:monetdb:
REJECT mapi:monetdb:/
```

```testNONONONONONONO
ACCEPT mapi:monetdb://
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
```

```test
ACCEPT mapi:monetdb://monet.db:12345/demo
EXPECT host=monet.db
EXPECT port=12345
EXPECT database=demo
EXPECT tls=off
EXPECT language=sql
```

```test
ACCEPT mapi:monetdb://localhost:12345/demo

```

```test
ACCEPT mapi:monetdb://monet.db:12345/demo?language=mal
EXPECT host=monet.db
EXPECT port=12345
EXPECT database=demo
EXPECT tls=off
EXPECT language=mal
```

```test
ACCEPT mapi:monetdb://monet.db/demo
EXPECT host=monet.db
EXPECT port=50000
EXPECT database=demo
EXPECT tls=off
EXPECT language=sql
```
Unix domain:


```test
ACCEPT mapi:monetdb:///path/to/socket
EXPECT host=
EXPECT sock=/path/to/socket
EXPECT database=
```

```test
ACCEPT mapi:monetdb:///path/to/socket?database=demo
EXPECT host=
EXPECT sock=/path/to/socket
EXPECT database=demo
```

Corner case: easy mistake to set sock to empty:

```test
ACCEPT mapi:monetdb:///
EXPECT host=
EXPECT sock=/
```
