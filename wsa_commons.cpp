#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
//#include "wsa_commons.h"
#include "wsa_error.h"

/**
 * Returns the error message based on the error ID given
 *
 * @param err_id The error ID
 */
const char *wsa_get_err_msg(int16_t err_id)
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