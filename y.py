#!/usr/bin/env python3

import fileinput
import re

mapping = dict(
    mapiparm                           = 'mparm',
    mapiparm_class                     = 'mparm_class',
    mapiparm_classify                  = 'mparm_classify',
    mapiparm_parse                     = 'mparm_parse',
    mapiparm_name                      = 'mparm_name',
    mapiparm_is_core                   = 'mparm_is_core',
    mapi_params                        = 'msettings',
    mapi_params_create                 = 'msettings_create',
    mapi_params_destroy                = 'msettings_destroy',
    mapi_param_string                  = 'msetting_string',
    mapi_param_set_string              = 'msetting_set_string',
    mapi_param_long                    = 'msetting_long',
    mapi_param_set_long                = 'msetting_set_long',
    mapi_param_bool                    = 'msetting_bool',
    mapi_param_set_bool                = 'msetting_set_bool',
    mapi_param_from_text               = 'msetting_parse',
    mapi_param_to_text                 = 'msetting_as_string',
    mapi_param_set_ignored             = 'msetting_set_ignored',
    mapi_param_set_named               = 'msetting_set_named',
    mapi_param_user_generation         = 'msettings_user_generation',
    mapi_param_password_generation     = 'msettings_password_generation',
    mapi_param_parse_url               = 'msettings_parse_url',
    mapi_param_validate                = 'msettings_validate',
    mapi_param_connect_scan            = 'msettings_connect_scan',
    mapi_param_connect_unix            = 'msettings_connect_unix',
    mapi_param_connect_tcp             = 'msettings_connect_tcp',
    mapi_param_connect_port            = 'msettings_connect_port',
    mapi_param_connect_tls_verify      = 'msettings_connect_tls_verify',
    mapi_param_connect_certhash_digits = 'msettings_connect_certhash_digits',
    mapi_param_connect_binary          = 'msettings_connect_binary',
    parse_bool                         = 'msetting_parse_bool',
)

pattern = '|'.join(mapping.keys())
pattern = re.compile(pattern)


for line in fileinput.input(encoding='utf-8', inplace=True):
    # new_line = pattern.sub(lambda m: mapping[m.group(0)], line)
    # new_line = line.replace('CP_', 'MP_')
    new_line = line.replace('CPT_', 'MPCLASS_')
    print(new_line, end='')
