#include <unistd.h>

#include "wsa_client.h"
#include "wsa_error.h"

/**
 * Close the connection
 *
 * @param cmd_sock -
 * @param data_sock -
 * 
 * @return 
 */
int16_t wsa_close_sock(int32_t sock_fd)
{
	// Close all socket file descriptors
	if (close(sock_fd) == -1)
		return WSA_ERR_SOCKETERROR;

	return 0;
}

void wsa_initialize_client()
{
	//Empty, since no initialization needs to be done
}

void wsa_destroy_client()
{
	//Empty, since nothing needs to be destroyed
}
