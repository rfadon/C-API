#ifndef __WSA_COMMONS_H__
#define __WSA_COMMONS_H__

#include "thinkrf_stdint.h"
#include "stdio.h"

#define FALSE	0
#define TRUE	1

#define MAX_FILE_LINES 300
#define SEP_CHARS "\n\r"

int16_t wsa_tokenize_file(FILE *fptr, char *cmd_str[]);
int16_t wsa_to_int(char const * num_str, int * val);
int16_t wsa_to_double(char const * num_str, double * val);
int16_t wsa_find_char_in_string(char const * string, char const * symbol);
#endif
