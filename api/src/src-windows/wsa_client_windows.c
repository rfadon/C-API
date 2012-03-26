#include <Ws2tcpip.h>

#include "thinkrf_stdint.h"
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

void wsa_initialize_client()
{
	struct WSAData ws_data;		// create an instance of Winsock data type
	int32_t ws_err_code;	// get error code

	if ((ws_err_code = WSAStartup(MAKEWORD(2, 2), &ws_data)) != 0) {
		doutf(DHIGH, "WSAStartup() returned error code %d. ", ws_err_code);
		//return WSA_ERR_WINSOCKSTARTUPFAILED;	// random # for now
	}
}

void wsa_destroy_client()
{
	WSACleanup();
}
