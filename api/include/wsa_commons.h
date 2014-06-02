#ifndef __WSA_COMMONS_H__
#define __WSA_COMMONS_H__

#include "thinkrf_stdint.h"

#define FALSE	0
#define TRUE	1

#define MAX_FILE_LINES 300
#define SEP_CHARS "\n\r"

int16_t wsa_tokenize_file(FILE *fptr, char *cmd_str[]);
int16_t wsa_to_int(char const * num_str, int *val);
int16_t to_double(char *num_str, double *val);
int16_t string_to_integer(const char *from_string, int32_t *to_integer);
int16_t find_char_in_string(char const *string, char *symbol);
#endif
