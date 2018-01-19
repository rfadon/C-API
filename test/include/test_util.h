
#include "thinkrf_stdint.h"
#include "wsa_lib.h"

void log_bug_result(struct test_data *test_info, char *print_string);

void verify_float_result(struct test_data *test_info, short function_result, float input_float, float output_float);

void verify_signed32_result(struct test_data *test_info, short function_result, int input_int, int output_int);

void verify_result(struct test_data *test_info, int16_t function_result, int fail_expected);

int16_t block_capture_tests(struct wsa_device *dev, struct test_data *test_info);

