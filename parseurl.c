#define _POSIX_C_SOURCE 200809L


#include "params.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Scanner state.
// Most scanner-related functions return 'false' on failure, 'true' on success.
// Some return a character pointer, NULL on failure, non-NULL on success.
typedef struct scanner {
	char *buffer;				// owned buffer with the scanned text in it
	char c;						// character we're currently looking at
	char *p;					// pointer to where we found c (may have been updated since)
	char error_message[256];	// error message, or empty string
} scanner;




static bool
initialize(scanner *sc, const char *url)
{
	sc->buffer = strdup(url);
	if (!sc->buffer)
		return false;
	sc->p = &sc->buffer[0];
	sc->c = *sc->p;
	sc->error_message[0] = '\0';
	return true;
}

static void
deinitialize(scanner *sc)
{
	free(sc->buffer);
}

static bool
has_failed(const scanner *sc)
{
	return sc->error_message[0] != '\0';
}

static char
advance(scanner *sc)
{
	assert(!has_failed(sc));
	sc->p++;
	sc->c = *sc->p;
	return sc->c;
}

static bool complain(scanner *sc, const char *fmt, ...)
	__attribute__((__format__(printf, 2, 3)));

static bool
complain(scanner *sc, const char *fmt, ...)
{
	// do not overwrite existing error message,
	// the first one is usually the most informative.
	if (!has_failed(sc)) {
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(sc->error_message, sizeof(sc->error_message), fmt, ap);
		va_end(ap);
		if (!has_failed(sc)) {
			// error message was empty, need non-empty so we know an error has occurred
			strcpy(sc->error_message, "?");
		}
	}

	return false;
}

static bool
unexpected(scanner *sc)
{
	if (sc->c == 0) {
		return complain(sc, "URL ended unexpectedly");
	} else {
		size_t pos = sc->p - sc->buffer;
		return complain(sc, "unexpected character '%c' at position %zu", sc->c, pos);
	}
}

static bool
consume(scanner *sc, const char *text)
{
	for (const char *c = text; *c; c++) {
		if (sc->c == *c) {
			advance(sc);
			continue;
		}
		size_t pos = sc->p - sc->buffer;
		if (sc->c == '\0') {
			return complain(sc, "unexpected end at position %zu, expected '%s'", pos, c);
		}
		return complain(sc, "unexpected character '%c' at position %zu, expected '%s'", sc->c, pos, c);
	}
	return true;
}


static int
percent_decode_digit(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A';
	if (c >= 'a' && c <= 'f')
		return c - 'a';
	// return something so negative that it will still
	// be negative after we combine it with another digit
	return -1000;
}

static bool
percent_decode(scanner *sc, const char *context, char *string)
{
	char *r = string;
	char *w = string;
	while (*r != '\0') {

		if (*r != '%') {
			*w++ = *r++;
			continue;
		}
		char x = r[1];
		if (x == '\0')
			return complain(sc, "percent escape in %s ends after one digit", context);
		char y = r[2];
		int n = 16 * percent_decode_digit(x) + percent_decode_digit(y);
		if (n < 0) {
			return complain(sc, "invalid percent escape in %s", context);
		}
		*w++ = (char)n;
		r += 3;

	}
	*w = '\0';
	return true;
}

enum character_class {
	// regular characters
	not_special,
	// special characters in the sense of RFC 3986 Section 2.2, plus '&' and '='
	generic_special,
	// very special, special even in query parameter values
	very_special,
};

static enum character_class
classify(char c)
{
	switch (c) {
		case '\0':
		case '#':
		case '&':
		case '=':
			return very_special;
		case ':':
		case '/':
		case '?':
		case '[':
		case ']':
		case '@':
			return generic_special;
		case '%':  // % is NOT special!
		default:
			return not_special;
	}
}

static char *
scan(scanner *sc, enum character_class level)
{
	assert(!has_failed(sc));
	char *token = sc->p;

	// scan the token
	while (classify(sc->c) < level)
		advance(sc);

	// the current character is a delimiter.
	// overwrite it with \0 to terminate the scanned string.
	assert(sc->c == *sc->p);
	*sc->p = '\0';

	return token;
}

static bool
store(mapi_params *mp, scanner *sc, mapiparm parm, const char *value)
{
	mapi_params_error msg = mapi_param_from_text(mp, parm, value);
	if (msg)
		return complain(sc, "cannot set %s to '%s': %s",mapiparm_name(parm), value, msg);
	else
		return true;
}


static bool
parse_modern(mapi_params *mp, scanner *sc)
{
	if (!consume(sc, "//"))
		return false;


	// parse the host
	if (sc->c == '[') {
		advance(sc);
		char *host = sc->p;
		while (sc->c == ':' || isxdigit(sc->c))
			advance(sc);
		*sc->p = '\0';
		if (!consume(sc, "]"))
			return false;
		if (!store(mp, sc, CP_HOST, host))
			return false;
	} else {
		char *host = scan(sc, generic_special);
		if (sc->c != ':' && sc->c != '/' && sc->c != '\0')
			return unexpected(sc);

		if (!percent_decode(sc, "host name", host))
			return false;
		if (sc->c == ':' && strlen(host) == 0) {
			// cannot port number without host, so this is not allowed: monetdb://:50000
			return unexpected(sc);
		}
		if (!store(mp, sc, CP_HOST, host))
			return false;
	}

	// parse the port
	if (sc->c == ':') {
		advance(sc);
		char *port = scan(sc, generic_special);
		if (!store(mp, sc, CP_PORT, port))
			return false;
	}

	// parse the database name
	if (sc->c == '/') {
		advance(sc);
		char *database = scan(sc, generic_special);
		if (!store(mp, sc, CP_DATABASE, database))
			return false;
	}

	// parse query parameters
	if (sc->c == '?') {
		do {
			advance(sc);
			char *key = scan(sc, very_special);
			if (strlen(key) == 0)
				return complain(sc, "parameter name must not be empty");
			if (!percent_decode(sc, "parameter name", key))
				return false;
			if (!consume(sc, "="))
				return false;
			char *value = scan(sc, very_special);
			if (!percent_decode(sc, key, value))
				return false;
			const mapiparm *parm = mapiparm_parse(key);
			if (parm != NULL) {
				if (!store(mp, sc, *parm, value))
					return false;
			} else if (mapiparm_is_ignored(key)) {
				mapi_params_error msg = mapi_param_set_ignored(mp, key, value);
				if (msg != NULL)
					return complain(sc, "cannot set '%s' to '%s': %s", key, value, msg);
			} else
				return complain(sc, "unknown parameter '%s'", key);
		} while (sc->c == '&');
	}

	// should have consumed everything
	if (sc->c != '\0' && sc-> c != '#')
		return unexpected(sc);

	return true;
}


static bool
parse_classic(mapi_params *mp, scanner *sc)
{
	if (!consume(sc, "monetdb://"))

	(void)mp;
	return complain(sc, "mapi: URLs are not supported yet");
}

static bool
parse(mapi_params *mp, scanner *sc)
{
	// process the scheme
	char *scheme = scan(sc, generic_special);
	if (sc->c == ':')
		advance(sc);
	else
		return complain(sc, "expected URL starting with monetdb:, monetdbs: or mapi:monetdb:");
	if (strcmp(scheme, "monetdb") == 0) {
		mapi_param_set_bool(mp, CP_TLS, false);
		return parse_modern(mp, sc);
	} else if (strcmp(scheme, "monetdbs") == 0) {
		mapi_param_set_bool(mp, CP_TLS, true);
		return parse_modern(mp, sc);
	} else if (strcmp(scheme, "mapi") == 0) {
		mapi_param_set_bool(mp, CP_TLS, true);
		return parse_classic(mp, sc);
	} else {
		return complain(sc, "unknown scheme '%s'", scheme);
	}
}


/* update the mapi_params from the URL. set *error_buffer to NULL and return true
 * if success, set *error_buffer to malloc'ed error message and return false on failure.
 * if return value is true but *error_buffer is NULL, malloc failed. */
bool mapi_param_parse_url(mapi_params *mp, const char *url, char **error_buffer)
{
	bool ok;
	scanner sc;
	if (!initialize(&sc, url))
		return false;

	// clear existing core values
	mapi_param_set_bool(mp, CP_TLS, false);
	mapi_param_set_string(mp, CP_HOST, "");
	mapi_param_set_long(mp, CP_PORT, 50000);
	mapi_param_set_string(mp, CP_DATABASE, "");

	if (parse(mp, &sc)) {
		// went well
		if (error_buffer)
			*error_buffer = NULL;
		ok = true;
	} else {
		// went wrong
		assert(sc.error_message[0] != '\0');
		char *msg = strdup(sc.error_message);
		if (error_buffer)
			*error_buffer = msg;
		ok = false;
	}

	deinitialize(&sc);
	return ok;
}
