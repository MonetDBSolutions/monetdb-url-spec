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

murltest: params.o parseurl.o testsfile.o murltest.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c params.h murltest.h
	$(CC) $(CFLAGS) -c $<

