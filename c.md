
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
PARSE mapi:monetdb://monet.db:12345/demo
EXPECT tls=off
```

