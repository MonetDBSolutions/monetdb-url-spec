MonetDB URL Specification
=========================

This repository contains the proposed specification for the new `monetdb://` /
`monetdbs://` style MonetDB URLs as opposed to the `mapi:monetdb://` URLs we
use right now.

Status: proposal.

The spec is in [monetdb-url.md][spec]. It is not finished yet.

There is a large, currently rather unstructured set of test cases in
[tests.md][testsmd]. The pymonetdb tests have a script that reads them from
[tests.md][testsmd] and tries them out. The other implementations will do the
same.

[spec]: monetdb-url.md
[testsmd]: tests.md