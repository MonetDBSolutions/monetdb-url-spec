# Tests

This document contains a large number of test cases.
They are embedded in the Markdown source, in <code>```test</code>
. . .</code>```</code> blocks.


The tests are written in a mini language with the following four
keywords:

* `PARSE url`: parse the URL, this should succeed.

* `REJECT url`: parse the URL, it should be rejected.

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
PARSE monetdb://localhost:50000/demo?user=monetdb&password=monetdb
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=demo
EXPECT user=monetdb
EXPECT password=monetdb
```

```test
PARSE monetdb:///demo
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=demo
```

```test
PARSE monetdb://localhost./demo
EXPECT connect_unix=
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=demo
```

```test
PARSE monetdb://localhost:50000/demo?user=monetdb&password=monetdb
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=demo
EXPECT user=monetdb
EXPECT password=monetdb
```

```test
PARSE monetdb://mdb.example.com:12345/demo
EXPECT connect_unix=
EXPECT connect_tcp=mdb.example.com
EXPECT port=12345
EXPECT tls=off
EXPECT database=demo
```

```test
PARSE monetdb://192.168.13.4:12345/demo
EXPECT connect_unix=
EXPECT connect_tcp=192.168.13.4
EXPECT port=12345
EXPECT tls=off
EXPECT database=demo
```

```test
PARSE monetdb://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:12345/demo
EXPECT connect_unix=
EXPECT connect_tcp=2001:0db8:85a3:0000:0000:8a2e:0370:7334
EXPECT port=12345
EXPECT tls=off
EXPECT database=demo
```

```test
PARSE monetdb://localhost/
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=
```

```test
PARSE monetdb://localhost
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
EXPECT port=50000
EXPECT tls=off
EXPECT database=
```

```test
PARSE monetdbs://mdb.example.com/demo
EXPECT connect_unix=
EXPECT connect_tcp=mdb.example.com
EXPECT port=50000
EXPECT tls=on
EXPECT connect_tls_verify=system
EXPECT database=demo
```

```test
PARSE monetdbs://mdb.example.com/demo?cert=/home/user/server.crt
EXPECT connect_unix=
EXPECT connect_tcp=mdb.example.com
EXPECT port=50000
EXPECT tls=on
EXPECT connect_tls_verify=cert
EXPECT cert=/home/user/server.crt
EXPECT database=demo
```

```test
PARSE monetdbs://mdb.example.com/demo?certhash={sha256}fb:67:20:aa:00:9f:33:4c
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
PARSE monetdbs://mdb.example.com/demo?certhash={sha256}A::B
EXPECT connect_certhash_algo=sha256
EXPECT connect_certhash_digits=ab
```

```test
PARSE monetdbs://mdb.example.com/demo?certhash={sHA256}A::B
EXPECT connect_certhash_algo=sha256
EXPECT connect_certhash_digits=ab
```

```test
PARSE monetdbs://mdb.example.com/demo?certhash={sHA1}A::B
EXPECT connect_certhash_algo=sha1
EXPECT connect_certhash_digits=ab
```

```test
PARSE monetdbs://mdb.example.com/demo?certhash=A::B
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
PARSE monetdb:///demo?sock=/var/monetdb/_sock&user=dbuser
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
PARSE monetdb:///?sock=/tmp/sock
EXPECT sock=/tmp/sock
PARSE monetdb:///?sock=C:\TEMP\sock
EXPECT sock=C:\TEMP\sock
```

### cert

```test
EXPECT cert=
PARSE monetdbs:///?cert=/tmp/cert.pem
EXPECT cert=/tmp/cert.pem
PARSE monetdbs:///?cert=C:\TEMP\cert.pem
EXPECT cert=C:\TEMP\cert.pem
```

### certhash

```test
EXPECT certhash=
PARSE monetdbs:///?certhash={sha1}001122ff
PARSE monetdbs:///?certhash={sha1}00:11:22:ff
PARSE monetdbs:///?certhash={sha256}001122ff
PARSE monetdbs:///?certhash=001122ff
REJECT monetdbs:///?certhash=sha1:001122ff
REJECT monetdbs:///?certhash={sha1}}001122gg
```

### clientkey, clientcert

```test
EXPECT clientkey=
PARSE monetdbs:///?clientkey=/tmp/clientkey.pem
EXPECT clientkey=/tmp/clientkey.pem
PARSE monetdbs:///?clientkey=C:\TEMP\clientkey.pem
EXPECT clientkey=C:\TEMP\clientkey.pem
```
### clientcert

```test
EXPECT clientcert=
PARSE monetdbs:///?clientcert=/tmp/clientcert.pem
EXPECT clientcert=/tmp/clientcert.pem
PARSE monetdbs:///?clientcert=C:\TEMP\clientcert.pem
EXPECT clientcert=C:\TEMP\clientcert.pem
```

### user, password

Not testing the default because they are (unfortunately)
implementation specific.

```test
PARSE monetdb:///?user=monetdb
EXPECT user=monetdb
PARSE monetdb:///?user=me&password=?
EXPECT user=me
EXPECT password=?
```

### language

```test
EXPECT language=sql
PARSE monetdb:///?language=msql
EXPECT language=msql
PARSE monetdb:///?language=sql
EXPECT language=sql
```

### autocommit

```test
PARSE monetdb:///?autocommit=true
EXPECT autocommit=true
PARSE monetdb:///?autocommit=on
EXPECT autocommit=true
PARSE monetdb:///?autocommit=yes
EXPECT autocommit=true
```

```test
PARSE monetdb:///?autocommit=false
EXPECT autocommit=false
PARSE monetdb:///?autocommit=off
EXPECT autocommit=false
PARSE monetdb:///?autocommit=no
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
PARSE monetdb:///?schema=foo
EXPECT schema=foo
PARSE monetdb:///?schema=
EXPECT schema=
PARSE monetdb:///?schema=foo
```

```test
PARSE monetdb:///?timezone=0
EXPECT timezone=0
PARSE monetdb:///?timezone=120
EXPECT timezone=120
PARSE monetdb:///?timezone=-120
EXPECT timezone=-120
REJECT monetdb:///?timezone=banana
```

### replysize and fetchsize

Note we never check `EXPECT fetchsize=`, it doesn't exist.

```test
PARSE monetdb:///?replysize=150
EXPECT replysize=150
PARSE monetdb:///?fetchsize=150
EXPECT replysize=150
PARSE monetdb:///?fetchsize=100&replysize=200
EXPECT replysize=200
PARSE monetdb:///?replysize=100&fetchsize=200
EXPECT replysize=200
```

### binary

```test
EXPECT binary=on
EXPECT connect_binary=65535
```

```test
PARSE monetdb:///?binary=on
EXPECT connect_binary=65535

PARSE monetdb:///?binary=yes
EXPECT connect_binary=65535

PARSE monetdb:///?binary=true
EXPECT connect_binary=65535

PARSE monetdb:///?binary=yEs
EXPECT connect_binary=65535
```

```test
PARSE monetdb:///?binary=off
EXPECT connect_binary=0

PARSE monetdb:///?binary=no
EXPECT connect_binary=0

PARSE monetdb:///?binary=false
EXPECT connect_binary=0
```

```test
PARSE monetdb:///?binary=0
EXPECT connect_binary=0

PARSE monetdb:///?binary=5
EXPECT connect_binary=5

PARSE monetdb:///?binary=0100
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
PARSE monetdb:///?ban_ana=bla
PARSE monetdb:///?hash=sha1
PARSE monetdb:///?debug=true
PARSE monetdb:///?logfile=banana
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
PARSE monetdb:///db1?schema=b
EXPECT schema=b
EXPECT database=db1
EXPECT tls=off
PARSE monetdbs:///db2?schema=c
EXPECT tls=on
EXPECT database=db2
EXPECT schema=c
```

Rule: a source that sets user must set password or clear.

```skiptest
PARSE monetdb:///?user=foo
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
PARSE monetdb:///?fetchsize=400
EXPECT replysize=400
PARSE monetdb:///?replysize=500&fetchsize=600
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
PARSE monetdb:///
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
PARSE monetdb://dbhost/dbdb
EXPECT tls=off
EXPECT host=dbhost
EXPECT port=50000
EXPECT database=dbdb
```

Careful around passwords

```test
SET user=alan
SET password=turing
PARSE monetdbs:///
EXPECT user=alan
EXPECT password=turing
```

```test
SET user=alan
SET password=turing
PARSE monetdbs:///?user=mathison
EXPECT user=mathison
EXPECT password=
```

The rule is, "if **user** changed", not "if **user** is set".

```test
SET user=alan
SET password=turing
PARSE monetdbs:///?user=alan
EXPECT user=alan
EXPECT password=turing
```

## URL syntax

General form

```test
PARSE monetdb://host:12345/db1/schema2/table3?user=mr&password=bean
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
PARSE monetdbs://h%6Fst:12345/db%31/schema%32/table%33?user=%6Dr&p%61ssword=bean
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
PARSE monetdb://host?user=claude&password=m%26ms
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
PARSE monetdbs:///?timezone=10&timezone=20
EXPECT timezone=20
```

Interesting case: setting user must clear the password but does
that also happen with repetitions within a URL?
Not sure. For the time being, no. This makes it easier for
situations where for example the query parameters come in
alphabetical order

```test
PARSE monetdb:///?user=foo&password=banana&user=bar
EXPECT user=bar
EXPECT password=banana
```

Similar but even simpler: user comes after password but does not
clear it.

```test
PARSE monetdb:///?password=pw&user=foo
EXPECT user=foo
EXPECT password=pw
```

Ways of writing booleans and the binary property have already been tested above.

Ip numbers:

```test
PARSE monetdb://192.168.1.1:12345/foo
EXPECT connect_unix=
EXPECT connect_tcp=192.168.1.1
EXPECT database=foo
```

```test
PARSE monetdb://[::1]:12345/foo
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
PARSE monetdb:///
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
```

```test
PARSE monetdb:///?sock=/a/path
EXPECT connect_unix=/a/path
EXPECT connect_tcp=
```

```test
PARSE monetdb://localhost/
EXPECT connect_unix=/tmp/.s.monetdb.50000
EXPECT connect_tcp=localhost
```

```test
PARSE monetdb://localhost/?sock=/a/path
EXPECT connect_unix=/a/path
EXPECT connect_tcp=
```

```test
PARSE monetdb://localhost./
EXPECT connect_unix=
EXPECT connect_tcp=localhost
```

```test
REJECT monetdb://localhost./?sock=/a/path
```

```test
PARSE monetdb://not.localhost/
EXPECT connect_unix=
EXPECT connect_tcp=not.localhost
```

```test
REJECT monetdb://not.localhost/?sock=/a/path
```

```test
PARSE monetdbs:///
EXPECT connect_unix=
EXPECT connect_tcp=localhost
```

```test
REJECT monetdbs:///?sock=/a/path
```

```test
PARSE monetdbs://localhost/
EXPECT connect_unix=
EXPECT connect_tcp=localhost
```

```test
REJECT monetdbs://localhost/?sock=/a/path
```

```test
PARSE monetdbs://localhost./
EXPECT connect_unix=
EXPECT connect_tcp=localhost
```

```test
REJECT monetdbs://localhost./?sock=/a/path
```

```test
PARSE monetdbs://not.localhost/
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


# Legacy URL's

```test
PARSE mapi:monetdb://monet.db:12345/demo
EXPECT host=monet.db
EXPECT port=12345
EXPECT database=demo
EXPECT tls=off
EXPECT language=sql
```

```test
PARSE mapi:monetdb://monet.db:12345/demo?language=mal
EXPECT host=monet.db
EXPECT port=12345
EXPECT database=demo
EXPECT tls=off
EXPECT language=mal
```

```test
PARSE mapi:monetdb://monet.db/demo
EXPECT host=monet.db
EXPECT port=50000
EXPECT database=demo
EXPECT tls=off
EXPECT language=sql
```
Unix domain:


```test
PARSE mapi:monetdb:///path/to/socket
EXPECT host=
EXPECT sock=/path/to/socket
EXPECT database=
```

```test
PARSE mapi:monetdb:///path/to/socket?database=demo
EXPECT host=
EXPECT sock=/path/to/socket
EXPECT database=demo
```

Corner case: easy mistake to set sock to empty:

```test
PARSE mapi:monetdb:///
EXPECT host=
EXPECT sock=/
```
