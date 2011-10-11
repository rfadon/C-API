#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
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
	return NULL;
}


/**
 * Tokenized all the words/strings in a file
 * Return pointer to the tokens
 */
int16_t wsa_tokenize_file(FILE *fptr, char *cmd_strs[])
{
	long fSize;
	char *buffer;
	char *fToken;
	int16_t next = 0;

	fseek(fptr, 0, SEEK_END);	// Reposition fptr posn indicator to the end ??
	fSize = ftell(fptr);		// obtain file size
	rewind(fptr);
	
	// allocate memory to contain the whole file:
	buffer = (char*) malloc (sizeof(char) * fSize);
	if (buffer == NULL) {
		fputs("Memory error", stderr); 	
		return WSA_ERR_MALLOCFAILED;
	}
	fread(buffer, 1, fSize, fptr);	// copy the file into the buffer
	//doutf(1, "\nFile content: \n%s\n", buffer);
	
	for (int i = 0; i < MAX_FILE_LINES; i++) 
		strcpy(cmd_strs[i], "");

	fToken = strtok(buffer, SEP_CHARS);
	while (fToken != NULL)	{
		//printf("%d fToken (%d) = %s\n", next, strlen(fToken), fToken);
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