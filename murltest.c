#define _POSIX_C_SOURCE 200809L

#include "testsfile.h"

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

char *USAGE = "Usage: murltest TESTFILES..";

bool
run_file(const char *filename)
{
	FILE *to_close, *f;
	if (strcmp(filename, "-") == 0) {
		f = stdin;
		to_close = NULL;
	} else {
		f = fopen(filename, "r");
		if (!f) {
			fprintf(stderr, "Could not open %s: %s\n", filename, strerror(errno));
			return false;
		}
		to_close = f;
	}

	bool ok = run_tests(filename, f);

	if (to_close)
		fclose(to_close);
	return ok;
}

int
main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "%s\n", USAGE);
		return 1;
	}

	bool ok = true;
	for (int i = 1; i < argc; i++) {
		ok &= run_file(argv[i]);
		if (!ok)
			break;
	}

	return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
