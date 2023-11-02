TESTS_MD=c.md tests.md

default: check testpy testc

clean:
	rm -f *.orig *.rej

check:
	python3 check.py
	markdownlint-cli2 monetdb-url.md tests.md
	echo

testpy:
	pytest test_target.py
	echo

testc:
	murltest -v $(TESTS_MD)

