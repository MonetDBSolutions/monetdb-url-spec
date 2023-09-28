#define _POSIX_C_SOURCE 200809L

#include "testsfile.h"
#include "params.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// love it when a task is so straightforward I can use global variables!
static int start_line = -1;
static mapi_params *mp = NULL;


static bool
handle_parse_command(const char *location, char *url)
{
	char *errmsg = NULL;
	bool ok = mapi_param_parse_url(mp, url, &errmsg);
	if (!ok) {
		assert(errmsg);
		fprintf(stderr, "%s: %s\n", location, errmsg);
		free(errmsg);
		return false;
	}

	// TODO validate!
	return true;
}

static bool
handle_reject_command(const char *location, char *url)
{
	bool ok = mapi_param_parse_url(mp, url, NULL);
	if (ok) {
		fprintf(stderr, "%s: expected URL to be rejected.\n", location);
		return false;
	}

	// TODO validate!
	return true;
}

static bool
handle_set_command(const char *location, char *key, char *value)
{
	const mapiparm *parm = mapiparm_parse(key);
	if (parm == NULL) {
		fprintf(stderr, "%s: unknown parameter '%s'\n", location, key);
		return false;
	}
	bool ok = mapi_param_from_text(mp, *parm, value);
	return ok;
}

static bool
expect_bool(const char *location, mapiparm parm, const char *value)
{
	int x = parse_bool(value);
	if (x < 0) {
		fprintf(stderr, "%s: syntax error: invalid bool '%s'\n", location, value);
	}
	bool b = x > 0;

	bool actual = mapi_param_bool(mp, parm);
	if (actual == b)
		return true;

	const char *name = mapiparm_name(parm);
	char *b_ = b ? "true" : "false";
	char *actual_ = actual ? "true" : "false";
	fprintf(stderr, "%s: expected %s=%s, found %s\n", location, name, b_, actual_);
	return false;

}

static bool
expect_long(const char *location, mapiparm parm, const char *value)
{
	if (strlen(value) == 0) {
		fprintf(stderr, "%s: syntax error: integer value cannot be empty string\n", location);
		return false;
	}
	char *end = NULL;
	long n = strtol(value, &end, 10);
	if (*end != '\0') {
		fprintf(stderr, "%s: syntax error: invalid integer '%s'\n", location, value);
		return false;
	}

	long actual = mapi_param_long(mp, parm);
	if (actual == n)
		return true;

	const char *name = mapiparm_name(parm);
	fprintf(stderr, "%s: expected %s=%ld, found %ld\n", location, name, n, actual);
	return false;
}

static bool
expect_string(const char *location, mapiparm parm, const char *value)
{
	const char *actual = mapi_param_string(mp, parm);
	if (strcmp(actual, value) == 0)
		return true;

	const char *name = mapiparm_name(parm);
	fprintf(stderr, "%s: expected %s='%s', found '%s'\n", location, name, value, actual);
	return false;
}


static bool
handle_expect_command(const char *location, char *key, char *value)
{
	const mapiparm *parm = mapiparm_parse(key);
	if (parm == NULL) {
		fprintf(stderr, "%s: unknown parameter '%s'\n:", location, key);
		return false;
	}

	if (*parm >= CP__BOOL_START && *parm < CP__BOOL_END)
		return expect_bool(location, *parm, value);
	if (*parm >= CP__LONG_START && *parm < CP__LONG_END)
		return expect_long(location, *parm, value);
	if (*parm >= CP__STRING_START && *parm < CP__STRING_END)
		return expect_string(location, *parm, value);
	fprintf(stderr, "%s: internal error\n", location);
	return false;
}




static bool
handle_line(int lineno, const char *location, char *line)
{
	// first trim trailing whitespace
	size_t n = strlen(line);
	while (n > 0 && isspace(line[n - 1]))
		n--;
	line[n] = '\0';

	if (mp == NULL) {
		// not in a code block
		if (strcmp(line, "```test") == 0) {
			// block starts here
			start_line = lineno;
			mp = mapi_params_create();
			if (mp != NULL)
				return true;
			fprintf(stderr, "%s: malloc failed\n", location);
			return false;
		} else {
			// ignore
			return true;
		}
	}

	// we're in a code block, does it end here?
	if (strlen(line) > 0 && line[0] == '`') {
		if (strcmp(line, "```") == 0) {
			// lone backticks, block ends here
			mapi_params_destroy(mp);
			mp = NULL;
			return true;
		} else {
			fprintf(stderr, "%s: unexpected backtick\n", location);
			return false;
		}
	}

	// this is line from a code block
	const char *whitespace = " \t";
	char *command = strtok(line, whitespace);
	if (command == NULL) {
		// empty line
		return true;
	} else if (strcasecmp(command, "PARSE") == 0) {
		char *url = strtok(NULL, "\n");
		if (url)
			return handle_parse_command(location, url);
	} else if (strcasecmp(command, "REJECT") == 0) {
		char *url = strtok(NULL, "\n");
		if (url)
			return handle_reject_command(location, url);
	} else if (strcasecmp(command, "SET") == 0) {
		char *key = strtok(NULL, "=");
		char *value = strtok(NULL, "\n");
		if (key)
			return handle_set_command(location, key, value ? value : "");
	} else if (strcasecmp(command, "EXPECT") == 0) {
		char *key = strtok(NULL, "=");
		char *value = strtok(NULL, "\n");
		if (key)
			return handle_expect_command(location, key, value ? value : "");
	} else {
		fprintf(stderr, "%s: unknown command: %s\n", location, command);
		return false;
	}

	// if we get here, url or key was not present
	fprintf(stderr, "%s: syntax error\n", location);
	return false;
}

bool
run_tests(const char *filename, FILE *f)
{
	char *location = malloc(strlen(filename) + 100);
	strcpy(location, filename);
	char *location_lineno = &location[strlen(filename)];
	*location_lineno++ = ':';
	*location_lineno = '\0';

	errno = 0;

	int lineno = 0;
	char *line_buffer = NULL;
	size_t line_buffer_size = 0;

	while (true) {
		lineno++;
		sprintf(location_lineno, "%d", lineno);
		ssize_t nread = getline(&line_buffer, &line_buffer_size, f);
		if (nread < 0) {
			if (errno) {
				fprintf(stderr, "%s: %s\n", location, strerror(errno));
				return false;
			} else {
				break;
			}
		}
		if (!handle_line(lineno, location, line_buffer))
			return false;
	}

	if (mp) {
		fprintf(stderr, "%s:%d: unterminated code block starts here\n", filename, start_line);
		return false;
	}

	return true;
}
