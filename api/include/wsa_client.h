#ifndef __WSA_CLIENT_H__
#define __WSA_CLIENT_H__

#include "thinkrf_stdint.h"

#define MAX_STR_LEN 512
#define MAX_BUF_SIZE 20

#define TIMEOUT 2000		/* Timeout for sockets in milliseconds */
#define CTRL_PORT "37001"
#define DATA_PORT "37000"

int16_t wsa_get_host_info(char *name);

int16_t wsa_addr_check(const char *sock_addr, const char *sock_port);
int16_t wsa_setup_sock(char *sock_name, const char *sock_addr, 
					   int32_t *sock_fd, const char *sock_port, int16_t timeout);
int16_t wsa_close_sock(int32_t sock_fd);

int32_t wsa_sock_send(int32_t sock_fd, char const *out_str, int32_t len);
int16_t wsa_sock_recv(int32_t sock_fd, uint8_t *rx_buf_ptr, int32_t buf_size,
					  uint32_t time_out, int32_t *bytes_received);
int16_t wsa_sock_recv_data(int32_t sock_fd, uint8_t *rx_buf_ptr, 
						   int32_t buf_size, uint32_t time_out, int32_t *total_bytes);
void wsa_initialize_client();
void wsa_destroy_client();

#endif
