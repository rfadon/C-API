
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wsa_api.h>
#include <wsa_lib.h>
#include <wsa_sweep_device.h>
#include <wsa_error.h>

int16_t attenuation_tests(struct wsa_device *dev, int32_t *fail_count, int32_t *pass_count);