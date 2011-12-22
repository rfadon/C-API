#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <errno.h>
#include <limits.h>

#include "wsa_commons.h"
#include "wsa_error.h"

/**
 * Returns the error message based on the error ID given
 *
 * @param err_id The error ID
 */
const char *_wsa_get_err_msg(int16_t err_id)
{
	int id = 0;

	// This loop is not efficient.  Should probably do a binary 
	// search instead but the error list is small right now.
	do
	{
		if (wsa_err_list[id].err_id == err_id) 
		{
			return wsa_err_list[id].err_msg;
			break;
		}
		else id++;
	} while (wsa_err_list[id].err_msg != NULL);
	
	return "unknown error code";
}


/**
 * Tokenized all the words/strings in a file.
 * Pointer to the tokens is stored in cmd_strs.
 * Return the number of tokens read.
 */
int16_t wsa_tokenize_file(FILE *fptr, char *cmd_strs[])
{
	long fSize;
	char *buffer;
	char *fToken;
	int16_t next = 0;
	int i;

	fseek(fptr, 0, SEEK_END);	// Reposition fptr posn indicator to the end
	fSize = ftell(fptr);		// obtain file size
	rewind(fptr);
	
	// allocate memory to contain the whole file:
	buffer = (char*) malloc (sizeof(char) * fSize);
	if (buffer == NULL) {
		fputs("Memory error", stderr); 	
		return WSA_ERR_MALLOCFAILED;
	}
	fread(buffer, 1, fSize, fptr);	// copy the file into the buffer
	doutf(DLOW, "\nFile content: \n%s\n", buffer);
	
	for (i = 0; i < MAX_FILE_LINES; i++) 
		strcpy(cmd_strs[i], "");

	fToken = strtok(buffer, SEP_CHARS);
	while (fToken != NULL)	{
		doutf(DLOW, "%d fToken (%d) = %s\n", next, strlen(fToken), fToken);
		// Avoid taking any empty line
		if (strpbrk(fToken, ":*?") != NULL) {
			strcpy(cmd_strs[next], fToken);
			next++;
		}
		fToken = strtok(NULL, SEP_CHARS);
	}

	free(buffer);

	return next;
}

/**
 * Convert a string to a long number type
 */
int16_t to_int(char *num_str, long int *val)
{
	char *temp;
	long int temp_val;

	errno = 0; // to distinguish success/failure after calling strtol
	temp_val = strtol(num_str, &temp, 0);
	if ((errno == ERANGE && (temp_val == LONG_MAX || temp_val == LONG_MIN))
		|| (errno != 0 && temp_val == 0)
		|| temp == num_str) {
		perror("strtol");
		return WSA_ERR_INVNUMBER;
	}

	*val = temp_val;

	return 0;
}

/**
 * Convert a string to a long number type
 */
int16_t to_double(char *num_str, double *val)
{
	char *temp;
	double temp_val;

	errno = 0; // to distinguish success/failure after calling strtol
	temp_val = strtod(num_str, &temp);
	if (errno == ERANGE || (errno != 0 && temp_val == 0) || temp == num_str) {
		perror("strtod");
		return WSA_ERR_INVNUMBER;
	}

	*val = temp_val;

	return 0;
}
