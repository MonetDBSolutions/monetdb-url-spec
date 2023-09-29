#define _POSIX_C_SOURCE 200809L

#include "params.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define FATAL() do { fprintf(stderr, "\n\n abort in params.c: %s\n\n", __func__); abort(); } while (0)

int parse_bool(const char *text)
{
	static struct { const char *word; bool value; } variants[] = {
		{ "true", true },
		{ "false", false },
		{ "yes", true },
		{ "no", false },
		{ "on", true },
		{ "off", false },
	};
	for (int i = 0; i < sizeof(variants) / sizeof(variants[0]); i++)
		if (strcasecmp(text, variants[i].word) == 0)
			return variants[i].value;
	return -1;
}


static const struct { const char *name;  mapiparm parm; }
by_name[] = {
	{ .name="autocommit", .parm=CP_AUTOCOMMIT },
	{ .name="binary", .parm=CP_BINARY },
	{ .name="cert", .parm=CP_CERT },
	{ .name="certhash", .parm=CP_CERTHASH },
	{ .name="clientcert", .parm=CP_CLIENTCERT },
	{ .name="clientkey", .parm=CP_CLIENTKEY },
	{ .name="database", .parm=CP_DATABASE },
	{ .name="host", .parm=CP_HOST },
	{ .name="language", .parm=CP_LANGUAGE },
	{ .name="password", .parm=CP_PASSWORD },
	{ .name="port", .parm=CP_PORT },
	{ .name="replysize", .parm=CP_REPLYSIZE },
	{ .name="fetchsize", .parm=CP_REPLYSIZE },
	{ .name="schema", .parm=CP_SCHEMA },
	{ .name="sock", .parm=CP_SOCK },
	{ .name="table", .parm=CP_TABLE },
	{ .name="tableschema", .parm=CP_TABLESCHEMA },
	{ .name="timezone", .parm=CP_TIMEZONE },
	{ .name="tls", .parm=CP_TLS },
	{ .name="user", .parm=CP_USER },
	//
	{ .name="hash", .parm=CP_IGNORE },
	{ .name="debug", .parm=CP_IGNORE },
	{ .name="logfile", .parm=CP_IGNORE },
};

const mapiparm
mapiparm_parse(const char *name)
{
	int n = sizeof(by_name) / sizeof(by_name[0]);
	// could use a binary search but this is not going to be a bottleneck
	for (int i = 0; i < n; i++)
		if (strcmp(by_name[i].name, name) == 0)
			return by_name[i].parm;

	return strchr(name, '_') ? CP_IGNORE : CP_UNKNOWN;
}

const char *
mapiparm_name(mapiparm parm)
{
	switch (parm) {
		case CP_AUTOCOMMIT: return "autocommit";
		case CP_BINARY: return "binary";
		case CP_CERT: return "cert";
		case CP_CERTHASH: return "certhash";
		case CP_CLIENTCERT: return "clientcert";
		case CP_CLIENTKEY: return "clientkey";
		case CP_DATABASE: return "database";
		case CP_HOST: return "host";
		case CP_LANGUAGE: return "language";
		case CP_PASSWORD: return "password";
		case CP_PORT: return "port";
		case CP_REPLYSIZE: return "replysize";
		case CP_SCHEMA: return "schema";
		case CP_SOCK: return "sock";
		case CP_TABLE: return "table";
		case CP_TABLESCHEMA: return "tableschema";
		case CP_TIMEZONE: return "timezone";
		case CP_TLS: return "tls";
		case CP_USER: return "user";
		default: FATAL();
	}
}

bool
mapiparm_is_core(mapiparm parm)
{
	switch (parm) {
		case CP_TLS:
		case CP_HOST:
		case CP_PORT:
		case CP_DATABASE:
		case CP_TABLESCHEMA:
		case CP_TABLE:
			return true;
		default:
			return false;
	}
}

struct mapi_params {
	bool bool_params[CP__BOOL_END - CP__BOOL_START];
	long long_params[CP__LONG_END - CP__LONG_START];
	char *string_parameters[CP__STRING_END - CP__STRING_START];
	char **unknown_parameters;
	size_t nr_unknown;
	long user_generation;
	long password_generation;
	char unix_sock_name_buffer[50];
	const char *certhash_algo;
	char certhash_digits_buffer[33];
	bool validated;
};


mapi_params *mapi_params_create(void)
{
	char *sql = strdup("sql");
	char *binary_on = strdup("on");
	mapi_params *mp = malloc(sizeof(*mp));
	if (!sql || !binary_on || !mp) {
		free(sql);
		free(binary_on);
		free(mp);
		return NULL;
	}
	*mp = (mapi_params) {
		.bool_params = {
			[CP_TLS - CP__BOOL_START] = false,
			[CP_AUTOCOMMIT - CP__BOOL_START] = true,
		},
		.long_params = {
			[CP_PORT - CP__LONG_START] = 50000,
			[CP_TIMEZONE - CP__LONG_START] = LONG_MIN,
			[CP_REPLYSIZE - CP__LONG_START] = 100,
		},
		.string_parameters = {
			[CP_HOST - CP__STRING_START] = NULL,
			[CP_DATABASE - CP__STRING_START] = NULL,
			[CP_TABLESCHEMA - CP__STRING_START] = NULL,
			[CP_TABLE - CP__STRING_START] = NULL,
			[CP_CERTHASH - CP__STRING_START] = NULL,
			[CP_USER - CP__STRING_START] = NULL,
			[CP_PASSWORD - CP__STRING_START] = NULL,
			[CP_LANGUAGE - CP__STRING_START] = sql,
			[CP_SCHEMA - CP__STRING_START] = NULL,
			[CP_BINARY - CP__STRING_START] = binary_on,
		},
		.unknown_parameters = NULL,
		.nr_unknown = 0,
		.validated = false,
	};
	return mp;
}

void
mapi_params_destroy(mapi_params *mp)
{
	for (int i = 0; i < sizeof(mp->string_parameters) / sizeof(mp->string_parameters[0]); i++)
		free(mp->string_parameters[i]);
	for (int i = 0; i < mp->nr_unknown; i++) {
		free(mp->unknown_parameters[2 * i]);
		free(mp->unknown_parameters[2 * i + 1]);
	}
	free(mp->unknown_parameters);
	free(mp);
}

const char*
mapi_param_string(const mapi_params *mp, mapiparm parm)
{
	if (parm < CP__STRING_START || parm >= CP__STRING_END)
		FATAL();
	char *s = mp->string_parameters[parm - CP__STRING_START];
	return s == NULL ? "" : s;
}


mapi_params_error
mapi_param_set_string(mapi_params *mp, mapiparm parm, const char* value)
{
	char *v;

	if (parm < CP__STRING_START || parm >= CP__STRING_END)
		FATAL();

	if (value && *value) {
		v = strdup(value);
		if (!v)
			return "malloc failed";
	} else {
		v = NULL;
	}

	char **slot = &mp->string_parameters[parm - CP__STRING_START];
	free(*slot);
	*slot = v;

	if (parm == CP_USER)
		mp->user_generation++;
	if (parm == CP_PASSWORD)
		mp->password_generation++;
	mp->validated = false;

	return NULL;
}


long
mapi_param_long(const mapi_params *mp, mapiparm parm)
{
	if (parm < CP__LONG_START || parm >= CP__LONG_END)
		FATAL();
	return mp->long_params[parm - CP__LONG_START];
}


mapi_params_error
mapi_param_set_long(mapi_params *mp, mapiparm parm, long value)
{
	if (parm < CP__LONG_START || parm >= CP__LONG_END)
		FATAL();
	if (parm == CP_PORT)
		if (value < 1 || value > 65535)
			return "port number out of range";
	mp->long_params[parm - CP__LONG_START] = value;
	mp->validated = false;
	return NULL;
}


bool
mapi_param_bool(const mapi_params *mp, mapiparm parm)
{
	if (parm < CP__BOOL_START || parm >= CP__BOOL_END)
		FATAL();
	return mp->bool_params[parm - CP__BOOL_START];
}


mapi_params_error
mapi_param_set_bool(mapi_params *mp, mapiparm parm, bool value)
{
	if (parm < CP__BOOL_START || parm >= CP__BOOL_END)
		FATAL();
	mp->bool_params[parm - CP__BOOL_START] = value;
	mp->validated = false;
	return NULL;
}

mapi_params_error
mapi_param_from_text(mapi_params *mp, mapiparm parm, const char *text)
{
	if (parm >= CP__BOOL_START && parm < CP__BOOL_END) {
		int b = parse_bool(text);
		if (b < 0)
			return "invalid boolean value";
		return mapi_param_set_bool(mp, parm, b);
	} else if (parm >= CP__LONG_START && parm < CP__LONG_END) {
		if (text[0] == '\0')
			return "integer parameter cannot be empty string";
		char *end;
		long l = strtol(text, &end, 10);
		if (*end != '\0')
			return "invalid integer";
		return mapi_param_set_long(mp, parm, l);
	} else if (parm >= CP__STRING_START && parm < CP__STRING_END) {
		return mapi_param_set_string(mp, parm, text);
	} else {
		FATAL();
	}
}

char *
mapi_param_to_text(mapi_params *mp, mapiparm parm)
{
	if (parm >= CP__BOOL_START && parm < CP__BOOL_END) {
		bool b = mapi_param_bool(mp, parm);
		return strdup(b ? "true" : " false");
	} else if (parm >= CP__LONG_START && parm < CP__LONG_END) {
		long l = mapi_param_long(mp, parm);
		int n = 40;
		char *buf = malloc(n);
		if (!buf)
			return NULL;
		snprintf(buf, n, "%ld", l);
		return buf;
	} else if (parm >= CP__STRING_START && parm < CP__STRING_END) {
		const char *s = mapi_param_string(mp, parm);
		return strdup(s);
	} else {
		FATAL();
	}
}

mapi_params_error
mapi_param_set_ignored(mapi_params *mp, const char *key, const char *value)
{
	char *my_key = strdup(key);
	char *my_value = strdup(value);

	size_t n = mp->nr_unknown;
	size_t new_size = (2 * n + 2) * sizeof(char*);
	char **new_unknowns = realloc(mp->unknown_parameters, new_size);

	if (!my_key || !my_value || !new_unknowns) {
		free(my_key);
		free(my_value);
		free(new_unknowns);
		return "malloc failed while setting ignored parameter";
	}

	new_unknowns[2 * n] = my_key;
	new_unknowns[2 * n + 1] = my_value;
	mp->unknown_parameters = new_unknowns;
	mp->nr_unknown += 1;

	return NULL;
}

/* store named parameter */
mapi_params_error
mapi_param_set_named(mapi_params *mp, bool allow_core, const char *key, const char *value)
{
	mapiparm parm = mapiparm_parse(key);
	if (parm == CP_UNKNOWN)
		return "unknown parameter";

	if (parm == CP_IGNORE)
		return mapi_param_set_ignored(mp, key, value);

	if (!allow_core && mapiparm_is_core(parm))
		return "parameter not allowed here";

	return mapi_param_from_text(mp, parm, value);
}


static bool
empty(const mapi_params *mp, mapiparm parm)
{
	if (parm >= CP__STRING_START && parm < CP__STRING_END) {
		const char *value = mapi_param_string(mp, parm);
		assert(value);
		return *value == '\0';
	}
	FATAL();
}

static bool
nonempty(const mapi_params *mp, mapiparm parm)
{
	return !empty(mp, parm);
}

static mapi_params_error
validate_certhash(mapi_params *mp)
{
	mp->certhash_algo = "sha1";
	mp->certhash_digits_buffer[0] = '\0';

	const char *certhash = mapi_param_string(mp, CP_CERTHASH);
	if (*certhash == '\0')
		return NULL;

	if (strncasecmp(certhash, "{sha1}", 6) == 0) {
		mp->certhash_algo = "sha1";
		certhash += 6;
	} else if (strncasecmp(certhash, "{sha256}", 8) == 0) {
		mp->certhash_algo = "sha256";
		certhash += 8;
	} else if (certhash[0] == '{') {
		return "certhash: invalid algorithm";
	}

	const int n = 32;
	assert(sizeof(mp->certhash_digits_buffer) >= n + 1);
	int i = 0;
	for (const char *r = certhash; *r; r++) {
		if (*r == ':')
			continue;
		if (!isxdigit(*r))
			return "certhash: invalid hex digit";
		if (i == n)
			return "certhash: too long";
		mp->certhash_digits_buffer[i++] = tolower(*r);
	}

	return NULL;
}

static bool
validate_identifier(const char *name)
{
	int first = name[0];
	if (first == '\0')
		return true;
	if (first != '_' && !isalpha(first))
		return false;
	for (const char *p = name; *p; p++) {
		bool ok = (isalnum(*p) || *p == '-' || *p == '_');
		if (!ok)
			return false;
	}
	return true;
}

mapi_params_error
mapi_param_validate(mapi_params *mp)
{
	if (mp->validated)
		return NULL;

	// 1. The parameters have the types listed in the table in [Section
	//    Parameters](#parameters).
	// (this has already been checked)

	// 2. If **sock** and **host** are both not empty, **host** must be equal
	//    to `localhost`.
	if (nonempty(mp, CP_SOCK) && nonempty(mp, CP_HOST))
		if (strcmp("localhost", mapi_param_string(mp, CP_HOST)) != 0)
			return "With sock=, host must be 'localhost'";

	// 3. The string parameter **binary** must either parse as a boolean or as a
	//    non-negative integer.
	mp->validated = true;
	long level = mapi_param_connect_binary(mp);
	mp->validated = false;
	if (level < 0)
		return "invalid value for parameter 'binary'";

	// 4. If **sock** is not empty, **tls** must be 'off'.
	if (nonempty(mp, CP_SOCK) && mapi_param_bool(mp, CP_TLS))
		return "TLS cannot be used with Unix domain sockets";

	// 5. If **certhash** is not empty, it must be of the form
	//    `hexdigits` or `{hashname}hexdigits` where hashname is 'sha1' or 'sha256'
	//    and hexdigits is a non-empty sequence of 0-9, a-f and underscores.
	const char *certhash_msg = validate_certhash(mp);
	if (certhash_msg)
		return certhash_msg;

	// 6. If **cert** or **certhash** are not empty, **tls** must be 'on'.
	if (nonempty(mp, CP_CERT) || nonempty(mp, CP_CERTHASH))
		if (!mapi_param_bool(mp, CP_TLS))
			return "'cert' and 'certhash' can only be used with monetdbs:";

	// 7. Parameter **database** must consist only of upper- and lowercase letters,
	//    digits, dashes and underscores. It must not start with a dash.
	//
	const char *database = mapi_param_string(mp, CP_DATABASE);
	if (!validate_identifier(database))
		return "invalid database name";
	const char *tableschema = mapi_param_string(mp, CP_TABLESCHEMA);
	if (!validate_identifier(tableschema))
		return "invalid schema name";
	const char *table = mapi_param_string(mp, CP_TABLE);
	if (!validate_identifier(table))
		return "invalid table name";

	// compute this here so the getter function can take const mapi_params*
	long port = mapi_param_long(mp, CP_PORT);
	snprintf(mp->unix_sock_name_buffer, sizeof(mp->unix_sock_name_buffer), "/tmp/.s.monetdb.%ld", port);

	mp->validated = true;
	return NULL;
}

const char *
mapi_param_connect_unix(const mapi_params *mp)
{
	assert(mp->validated);
	const char *sock = mapi_param_string(mp, CP_SOCK);
	const char *host = mapi_param_string(mp, CP_HOST);
	bool tls = mapi_param_bool(mp, CP_TLS);

	if (*sock)
		return sock;
	if (tls)
		return "";
	if (*host == '\0' || strcmp(host, "localhost") == 0)
		return mp->unix_sock_name_buffer;
	return "";
}


const char *
mapi_param_connect_tcp(const mapi_params *mp)
{
	assert(mp->validated);
	const char *sock = mapi_param_string(mp, CP_SOCK);
	const char *host = mapi_param_string(mp, CP_HOST);
	// bool tls = mapi_param_bool(mp, CP_TLS);

	if (*sock)
		return "";
	if (!*host || strcmp(host, "localhost.") == 0)
		return "localhost";
	return host;
}

const char*
mapi_param_connect_tls_verify(const mapi_params *mp)
{
	assert(mp->validated);
	bool tls = mapi_param_bool(mp, CP_TLS);
	const char *cert = mapi_param_string(mp, CP_CERT);
	const char *certhash = mapi_param_string(mp, CP_CERTHASH);

	if (!tls)
		return "";
	if (*certhash)
		return "hash";
	if (*cert)
		return "cert";
	return "system";
}

const char*
mapi_param_connect_certhash_algo(const mapi_params *mp)
{
	return mp->certhash_algo;
}

const char*
mapi_param_connect_certhash_digits(const mapi_params *mp)
{
	return mp->certhash_digits_buffer;
}

// also used as a validator, returns < 0 on invalid
long
mapi_param_connect_binary(const mapi_params *mp)
{
	const char *binary = mapi_param_string(mp, CP_BINARY);

	// must not be empty
	if (binary[0] == '\0')
		return -1;

	// may be bool
	int b = parse_bool(binary);
	if (b == 0)
		return 0;
	if (b == 1)
		return 65535; // "sufficiently large"
	assert(b < 0);

	char *end;
	long level = strtol(binary, &end, 10);
	if (*end == '\0')
		return level;

	return -1;
}


/* automatically incremented each time the corresponding field is updated */
long
mapi_param_user_generation(const mapi_params *mp)
{
	return mp->user_generation;
}

/* automatically incremented each time the corresponding field is updated */
long
mapi_param_password_generation(const mapi_params *mp)
{
	return mp->password_generation;
}
