# Implementation in JDBC

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

As described in Section&nbsp;[Syntax](#syntax), the fields **tls**,
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

