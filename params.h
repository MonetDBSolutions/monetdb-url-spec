#include <stdbool.h>


typedef enum mapiparm {

        // bool
        CP_TLS,
        #define CP__BOOL_START CP_TLS
        CP_AUTOCOMMIT,
        #define CP__BOOL_END CP__LONG_START

        // long
        CP_PORT,
        #define CP__LONG_START CP_PORT
        CP_TIMEZONE,
        CP_REPLYSIZE,
        #define CP__LONG_END CP__STRING_START

        // string
        CP_SOCK,
        #define CP__STRING_START CP_SOCK
        CP_CERT,
        CP_CLIENTKEY,
        CP_CLIENTCERT,
        CP_HOST,
        CP_DATABASE,
        CP_TABLESCHEMA,
        CP_TABLE,
        CP_CERTHASH,
        CP_USER,
        CP_PASSWORD,
        CP_LANGUAGE,
        CP_SCHEMA,
        CP_BINARY,
        #define CP__STRING_END CP__NUMBER_OF_PARAMETERS

        CP__NUMBER_OF_PARAMETERS,
} mapiparm;


/* returns NULL if not found, pointer to mapiparm if found */
const mapiparm *mapiparm_parse(const char *name);
const char *mapiparm_name(mapiparm parm);
bool mapiparm_is_ignored(const char *name);

typedef struct mapi_params mapi_params;

/* NULL means OK. non-NULL is error message. Valid until next call. Do not free. */
typedef const char *mapi_params_error;

/* returns NULL if could not allocate */
mapi_params *mapi_params_create(void);
void mapi_params_destroy(mapi_params *mp);

/* retrieve and set; call abort() on type error */

const char* mapi_param_string(const mapi_params *mp, mapiparm parm);
mapi_params_error mapi_param_set_string(mapi_params *mp, mapiparm parm, const char* value);

long mapi_param_long(const mapi_params *mp, mapiparm parm);
mapi_params_error mapi_param_set_long(mapi_params *mp, mapiparm parm, long value);

bool mapi_param_bool(const mapi_params *mp, mapiparm parm);
mapi_params_error mapi_param_set_bool(mapi_params *mp, mapiparm parm, bool value);

/* parse into the appropriate type, or format into newly malloc'ed string (NULL means malloc failed) */
mapi_params_error mapi_param_from_text(mapi_params *mp, mapiparm parm, const char *text);
char *mapi_param_to_text(mapi_params *mp, mapiparm parm);

/* store ignored parameter */
mapi_params_error mapi_param_set_ignored(mapi_params *mp, const char *key, const char *value);


/* update the mapi_params from the URL. set *error_buffer to NULL and return true
 * if success, set *error_buffer to malloc'ed error message and return false on failure.
 * if return value is true but *error_buffer is NULL, malloc failed. */
bool mapi_param_parse_url(mapi_params *mp, const char *url, char **error_buffer);

/* 1 = true, 0 = false, -1 = could not parse */
int parse_bool(const char *text);

/* return an error message if the validity rules are not satisfied */
mapi_params_error mapi_param_validate(mapi_params *mp);


/* virtual parameters */
const char *mapi_param_connect_unix(const mapi_params *mp);
const char *mapi_param_connect_tcp(const mapi_params *mp);
const char *mapi_param_connect_tls_verify(const mapi_params *mp);
const char *mapi_param_connect_certhash_algo(const mapi_params *mp);
const char *mapi_param_connect_certhash_digits(const mapi_params *mp);
long mapi_param_connect_binary(const mapi_params *mp);

/* automatically incremented each time the corresponding field is updated */
long mapi_param_user_generation(const mapi_params *mp);
long mapi_param_password_generation(const mapi_params *mp);
