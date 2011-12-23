#ifndef __WSA_CLIENT_H__
#define __WSA_CLIENT_H__

#include "stdint.h"
#include <winsock2.h>

#define MAX_STR_LEN 512
#define MAX_BUF_SIZE 20

#define TIMEOUT 500		/* Timeout for sockets in milliseconds */


#ifdef WIN_SOCK

#define CTRL_PORT 37001	/* Connection protocol's port to use with TCPIP */
#define DATA_PORT 37000



///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
uint32_t wsa_addr_check(const char *sock_addr);
int16_t wsa_get_host_info(char *name);

int16_t wsa_start_client(const char *wsa_addr, SOCKET *cmd_sock, 
					SOCKET *data_sock, int32_t ctrl_port, int32_t data_port);
int16_t wsa_close_client(SOCKET cmd_sock, SOCKET data_sock);

int32_t wsa_sock_send(SOCKET out_sock, char *out_str, int32_t len);
int32_t wsa_sock_recv(SOCKET in_sock, char *rx_buf_ptr, uint32_t buf_size,
					  uint32_t time_out);
int32_t wsa_sock_recv_data(SOCKET in_sock, char *rx_buf_ptr, uint32_t buf_size,
					  uint32_t time_out);
#else

#define CTRL_PORT "37001"
#define DATA_PORT "37000"

int16_t wsa_get_host_info(char *name);

int16_t wsa_addr_check(const char *sock_addr, const char *sock_port);
int16_t wsa_setup_sock(char *sock_name, const char *sock_addr, 
					   int32_t *sock_fd, const char *sock_port);
int16_t wsa_close_sock(int32_t sock_fd);

int32_t wsa_sock_send(int32_t sock_fd, char *out_str, int32_t len);
int32_t wsa_sock_recv(int32_t sock_fd, char *rx_buf_ptr, uint32_t buf_size,
					  uint32_t time_out);
int32_t wsa_sock_recv_data(int32_t sock_fd, char *rx_buf_ptr, 
						   uint32_t buf_size, uint32_t time_out);

#endif

#endif

