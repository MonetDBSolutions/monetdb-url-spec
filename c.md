
```no
PARSE banana
```

```no
PARSE banana:
```

```no
PARSE monetdb:/
```

```no
PARSE monetdb://joeri:nienke
```

```no
PARSE monetdb://joeri:12345:
```

```no
PARSE mapi
PARSE mapi:monetdb
PARSE mapi:monetdb:
PARSE mapi:monetdb:/
PARSE mapi:monetdb://
```

```no
PARSE monetdb://[abc]
```

```test
REJECT monetdb://foo:1/bar?tls=off
REJECT monetdb://foo:1/bar?host=localhost
REJECT monetdb://foo:1/bar?port=12345
REJECT monetdb://foo:1/bar?database=allmydata
REJECT monetdb://foo:1/bar?tableschema=banana
REJECT monetdb://foo:1/bar?table=tabularity
```
