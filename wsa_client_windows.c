#include <stdint.h>
#include <Ws2tcpip.h>

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
	if (closesocket(sock_fd) == -1)
		return WSA_ERR_SOCKETERROR;

	return 0;
}