#ifndef __WSA_CLIENT_H__
#define __WSA_CLIENT_H__

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "stdint.h"
#include "ws-util.h"

#define MAX_STR_LEN 200
#define MAX_BUF_SIZE 20

#define TIMEOUT 1000	/* Timeout for sockets in milliseconds */
#define HISLIP 4880		/* Connection protocol's port to use with TCPIP */


///////////////////////////////////////////////////////////////////////////////
// Global Functions
///////////////////////////////////////////////////////////////////////////////
int16_t wsa_list_ips(char **ip_list);
u_long wsa_verify_addr(const char *sock_addr);
int16_t wsa_get_host_info(char *name);
int16_t wsa_start_client(const char *wsa_addr, SOCKET *cmd_sock, 
						 SOCKET *data_sock);
int16_t wsa_close_client(SOCKET cmd_sock, SOCKET data_sock);
int16_t wsa_sock_send(SOCKET out_sock, char *out_str, int32_t len);
int64_t wsa_sock_recv(SOCKET in_sock, char *rx_buf_ptr, uint32_t time_out);

#endif

