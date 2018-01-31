#include "thinkrf_stdint.h"
#include "wsa_lib.h"


struct test_data {

	// total test count
	int test_count;

	// total bug count
	int bug_count;

	// total fail count
	int fail_count;

	// total pass count
	int pass_count;

	// flag to indicate whether current bug is expected to fail (0 expected to pass, 1 expected to fail)
	int fail_expected;

};

void init_test_data(struct test_data *test_info);
void log_bug_result(struct test_data *test_info, char *print_string);
void verify_float_result(struct test_data *test_info, short function_result, float input_float, float output_float);
void verify_signed32_result(struct test_data *test_info, short function_result, int input_int, int output_int);
void verify_result(struct test_data *test_info, int16_t function_result, int fail_expected);


int16_t test_device_descr(struct wsa_device *dev, struct test_data *test_info);
int16_t attenuation_tests(struct wsa_device *dev, struct test_data *test_info);
int16_t sweep_attenuation_tests(struct wsa_device *dev, struct test_data *test_info);
int16_t freq_tests(struct wsa_device *dev, struct test_data *test_info);
int16_t sweep_freq_tests(struct wsa_device *dev, struct test_data *test_info);
int16_t sweep_device_tests(struct wsa_device *dev, struct test_data *test_info);
int16_t block_capture_tests(struct wsa_device *dev, struct test_data *test_info);
int16_t stream_tests(struct wsa_device *dev, struct test_data *test_info);
int16_t sweep_tests(struct wsa_device *dev, struct test_data *test_info);
