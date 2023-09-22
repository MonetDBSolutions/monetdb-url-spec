# Implementation in libmapi / mclient / ODBC

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

