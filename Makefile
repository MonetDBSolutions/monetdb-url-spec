CC=gcc
CFLAGS=-std=c99 -Wall -Werror -ggdb3

default: check testpy testc

clean:
	rm -f murltest *.o

check:
	python3 check.py
	markdownlint-cli2 monetdb-url.md
	echo

testpy:
	pytest test_target.py
	echo

testc: murltest
	./murltest c.md tests.md
	echo

murltest: murltest.o testsfile.o parseurl.o params.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c params.h testsfile.h
	$(CC) $(CFLAGS) -c $<

