#include <stdio.h>
#include "thinkrf_stdint.h"
#include "test_util.h"
void log_bug_result(struct test_data *test_info, char *print_string){
	int x = 5;
}



void verify_float_result(struct test_data *test_info, int16_t function_result, float input_float, float output_float){
	int x = 3;
}

// verify test results of 32-bit queries
void verify_signed32_result(struct test_data *test_info, int16_t function_result, int32_t input_int, int32_t output_int){

	if (function_result < 0){
		test_info->fail_count = test_info->fail_count + 1;
		return;
	}
	else{
		
		if (input_int != output_int)
			test_info->fail_count = test_info->fail_count + 1;
		else
			test_info->pass_count = test_info->pass_count + 1;
	}
}

// verify test results from the error code returned from C-API functions
void verify_result(struct test_data *test_info, int16_t function_result, int fail_expected){

	// determine if the function has failed
	if (function_result < 0){

		// determine if the function was expected to fail, and failed, add a pass count
		if (fail_expected == 1){
			test_info->pass_count = test_info->pass_count + 1;
			return;
		}
		
		// if the test was expected to pass, and it failed, add a fail count
		else{
			test_info->fail_count = test_info->fail_count + 1;
			return;
		}
	}
	
	// condition of the test has passed
	else{
		// if the test was expected to fail, but passed, adda  fail count
		if (fail_expected == 1){
			test_info->fail_count = test_info->fail_count + 1;
			return;
		}

		// if the test was expected to pass, and passed, add pass count
		else{
			test_info->pass_count = test_info->pass_count + 1;
			return;
		}
	}
}

