#include <stdio.h>
#include "thinkrf_stdint.h"
#include "test_util.h"


void init_test_data(struct test_data *test_info) {
	test_info->test_count = 0;
	test_info->bug_count = 0;
	test_info->fail_count = 0;
	test_info->pass_count = 0;
	test_info->fail_count = 0;
}


void log_bug_result(struct test_data *test_info, char *print_string) {
	// TODO
}


void verify_float_result(struct test_data *test_info, int16_t function_result, float input_float, float output_float) {
	test_info->test_count++;

	if (function_result < 0) {
		test_info->fail_count++;
	}
	else {
		if (input_float != output_float) {
			printf("Input does not equal output: %f %f\n", input_float, output_float);
			test_info->fail_count++;
		}
		else {
			test_info->pass_count++;
		}
	}

	return;
}


// verify test results of 32-bit queries
void verify_signed32_result(struct test_data *test_info, int16_t function_result, int32_t input_int, int32_t output_int) {
	test_info->test_count++;

	if (function_result < 0) {
		test_info->fail_count++;
	}
	else {

		if (input_int != output_int)
			test_info->fail_count++;
		else
			test_info->pass_count++;
	}

	return;
}


// Verify test results from the error code returned from C-API functions
void verify_result(struct test_data *test_info, int16_t function_result, int fail_expected) {
	test_info->test_count++;

	// Determine if the function has produced the expected results
	if (function_result < 0) {
		// Test failed
		if (fail_expected) {
			// Test failed as expected.
			test_info->pass_count++;
		}
		else {
			// It failed but should have passed.
			test_info->fail_count++;
			printf("Expected pass but got failure!\n");
		}
	} else {
		// Test passed
		if (fail_expected) {
			// Expected failure but test passed.
			test_info->fail_count++;
			printf("Expected fail but got pass!\n");
		} else {
			// Expected pass and got it.
			test_info->pass_count++;
		}
	}
	return;
}


