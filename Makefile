
# CC=gcc
# CFLAGS=-std=c99 -Wall -Werror -ggdb3
CC=clang
CFLAGS=-std=c99 -Wall -Werror -ggdb3 -fsanitize=address,undefined
SRC_C=params.c parseurl.c testsfile.c murltest.c
FUZZ_C=fuzzer.c parseurl.c params.c
SRC_H=params.h murltest.h
TESTS_MD=c.md tests.md

default: check testpy testc

clean:
	rm -f murltest murlcov murlfuzz *.o *.gcda *.gcno *.gcov

check:
	python3 check.py
	markdownlint-cli2 monetdb-url.md tests.md
	echo

testpy:
	pytest test_target.py
	echo

testc: murltest
	./murltest -v $(TESTS_MD)

murltest: $(SRC_C:.c=.o)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(SRC_H)
	$(CC) $(CFLAGS) -c $<

coverage: murlcov
	./murlcov $(TESTS_MD)
	gcov -k murlcov-params

murlcov: $(SRC_C) $(SRC_H)
	gcc -std=c99  -fprofile-arcs -ftest-coverage -o $@ $(SRC_C)

murlfuzz: $(FUZZ_C) $(SRC_H)
	clang -Wall -Werror -std=c99 -g -fsanitize=address,fuzzer,undefined -o $@ $(FUZZ_C)
