#define _POSIX_C_SOURCE 200809L

#include "params.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int countcountcount = 0;


int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
	mapi_params *mp = mapi_params_create();

	const char *data = (const char*)Data;
	size_t len = strnlen(data, Size);
	char *buf = malloc(len + 1);
	if (!mp || !buf)
		return 0;


	memcpy(buf, data, len);
	buf[len] = '\0';

	char *errmsg = NULL;

	if (mapi_param_parse_url(mp, buf, &errmsg)) {
		if (mapi_param_validate(mp))
			countcountcount++;
	}

	mapi_params_destroy(mp);
	free(buf);
	free(errmsg);
  return 0;
}
