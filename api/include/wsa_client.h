#ifndef __WSA_CLIENT_H__
#define __WSA_CLIENT_H__

#include "stdint.h"

#define MAX_STR_LEN 512
#define MAX_BUF_SIZE 20

#define TIMEOUT 1000		/* Timeout for sockets in milliseconds */
#define CTRL_PORT "37001"
#define DATA_PORT "37000"

int16_t wsa_get_host_info(char *name);

int16_t wsa_addr_check(const char *sock_addr, const char *sock_port);
int16_t wsa_setup_sock(char *sock_name, const char *sock_addr, 
					   int32_t *sock_fd, const char *sock_port);
int16_t wsa_close_sock(int32_t sock_fd);

int32_t wsa_sock_send(int32_t sock_fd, char *out_str, int32_t len);
int32_t wsa_sock_recv(int32_t sock_fd, char *rx_buf_ptr, int32_t buf_size,
					  uint32_t time_out);
int32_t wsa_sock_recv_data(int32_t sock_fd, char *rx_buf_ptr, 
						   int32_t buf_size, uint32_t time_out);
void wsa_initialize_client();
void wsa_destroy_client();

#endif
