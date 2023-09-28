
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

```test
PARSE monetdb://localhost:50000/demo?user=monetdb&password=monetdb
EXPECT port=50000
EXPECT tls=off
EXPECT database=demo
EXPECT user=monetdb
EXPECT password=monetdb
```
