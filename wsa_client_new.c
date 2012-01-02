
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "wsa_client.h"


///////////////////////////////////////////////////////////////////////////////
// Local Prototypes
///////////////////////////////////////////////////////////////////////////////
void *get_in_addr(struct sockaddr *sock_addr);
int16_t _addr_check(const char *sock_addr, const char *sock_port,
					struct addrinfo *ai_list);


/**
 * Get sockaddr, IPv4 or IPv6
 * 
 * Source: http://beej.us
 */
void *get_in_addr(struct sockaddr *sock_addr)
{
	if (sock_addr->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sock_addr)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sock_addr)->sin6_addr);
}


/**
 * Local function that does address check with all the proper socket setup and
 * call getaddrinfo() to verify.
 */
int16_t _addr_check(const char *sock_addr, const char *sock_port,
					struct addrinfo *ai_list)
{
	// ******
	// Short notations used:
	// fd = file descriptor
	// ai = address info
	// *****
	struct addrinfo hint_ai, *ai_ptr; 
	int16_t result;

	// Construct local address structure
	memset(&hint_ai, 0, sizeof(hint_ai)); //Zero out structure
	hint_ai.ai_family = AF_UNSPEC;		// Address family unspec in order to
										// return socket addresses for any 
										// address family (IPv4, IPv6, etc.)
	hint_ai.ai_socktype = SOCK_STREAM;	// For TCP type. Or use 0 for any type
	hint_ai.ai_flags = 0;
	hint_ai.ai_protocol = 0				// to auto chose the protocol

	// Check the address at the given port
	result = getaddrinfo(sock_addr, sock_port, &hint_ai, &ai_list);
	if (result != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
		return WSA_ERR_INVIPHOSTADDRESS;
	}

	return 0;
}

/**
 * Given a client address string and the port number, determine if it's 
 * a dotted-quad IP address or a domain address.
 *
 * @param sock_addr - 
 * @param sock_port -
 *
 * @return 0 upon successful, or a negative value when failed.
 */
int16_t wsa_addr_check(const char *sock_addr, const char *sock_port)
{
	struct addrinfo *ai_list;
	int16_t result;

	// Check the address at the given port
	result = _addr_check(sock_addr, sock_port, &hint_ai, &ai_list);
	if (result != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
		return WSA_ERR_INVIPHOSTADDRESS;
	}

	freeaddrinfo(ai_list);

	return 0;
}

/**
 * Look up, verify and establish the socket once deemed valid
 *
 * @param sock_name - Name of the socket (ex. server, client)
 * @param sock_addr -
 * @param sock_fd -
 * @param sock_port -
 *
 * @return Newly-connected socket when succeed, or INVALID_SOCKET when fail.
 */
int16_t wsa_setup_sock(char *sock_name, const char *sock_addr, 
					   int32_t *sock_fd, const char *sock_port)
{
	printf("setting up %s socket at port %s ... ", sock_name, sock_port);

	struct addrinfo *ai_list, *ai_ptr;
	int16_t result;	
	int32_t temp_fd;

	result = _addr_check(sock_addr, sock_port, ai_list);
	if (result < 0)
		return result;

	// loop through all the results and bind to the first we can
	for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
		// Check to find a valid socket
		temp_fd = socket(ai_ptr->ai_family, ai_ptr->ai_socktype,
			ai_ptr->ai_protocol);
		if (temp_fd == -1) {
			perror("client: socket() error");
			continue;
		}

		// establish the client connection
		if (connect(temp_fd, ai_ptr->ai_addr, ai_ptr->ai_addrlen) == -1) {
			close(temp_fd);
			perror("client: connect() error");
			continue;
		}

		break; // successfully connected if got to here
	}

	// If no address succeeded
	if (ai_ptr == NULL)  {
		fprintf(stderr, "client: failed to connect\n");
		return WSA_ERR_ETHERNETCONNECTFAILED;
	}
	
	char str[INET6_ADDRSTRLEN];
	inet_ntop(ai_ptr->ai_family, get_in_addr(
		(struct sockaddr *) ai_ptr->ai_addr), str, sizeof(str));
	printf("connected to %s\n", str);

	freeaddrinfo(ai_list); // all done with this list

	*sock_fd = temp_fd;
	
	return 0;
}

/**
 * Close the connection
 *
 * @param cmd_sock -
 * @param data_sock -
 * 
 * @return 
 */
int16_t wsa_close_client(int32_t sock_fd)
{
	// Close all socket file descriptors
	if (close(sock_fd) == -1)
		return WSA_ERR_SOCKETERROR;

	return 0;
}

/**
 * Sends a string to the server.  
 *
 * @param sock_fd -
 * @param out_str -
 * @param len -
 * 
 * @returns Number of bytes sent on success, or negative otherwise.
 */
int32_t wsa_sock_send(int32_t sock_fd, char *out_str, int32_t len)
{
	int32_t total_txed = 0;
	int32_t bytes_txed;
	int32_t bytes_left = len;

	// Loop to send all the bytes
	while (total_txed < len) {
		// Send the bytes
		bytes_txed = send(sock_fd, out_str + total_txed, bytes_left, 0);
		
		// Check the returned value
		if (bytes_txed > 0) {
			doutf(DMED, "Sent %d bytes to server.\n", bytes_txed);
			
			// update all the count
			total_txed += bytes_txed;
			bytes_left -= bytes_txed;
		}
		else if (bytes_txed == -1)
			return WSA_ERR_SOCKETERROR;
		else {
			// Client closed connection before we could reply to
			// all the data it sent, so bomb out early.
			return WSA_ERR_SOCKETDROPPED;
		}
	}

	return total_txed;
}


/**
 * Gets incoming control strings from the server socket \b buf_size bytes 
 * at a time.  It does not loop to keep checking \b buf_size of bytes are
 * received.
 *
 * @param sock_fd - The socket at which the data will be received.
 * @param rx_buf_ptr - A char pointer buffer to store the incoming bytes.
 * @param buf_size - The size of the buffer in bytes.
 * @param time_out - Time out in milliseconds.
 * 
 * @return Number of bytes read
 */
int32_t wsa_sock_recv(int32_t sock_fd, char *rx_buf_ptr, uint32_t buf_size,
					  uint32_t time_out)
{
	int32_t bytes_rxed = 0;

	// TODO make this non-blocking
	bytes_rxd = recv(ctrl_sock_fd, rx_buf, MAX_PKT_SIZE, 0);
	if (bytes_rxed == SOCKET_ERROR) {
		doutf(DMED, "recv() function returned with error %d\n", 
			WSAGetLastError());
		return WSA_ERR_SOCKETERROR;
	}

	// Terminate the last character in cmd resp string only to 0
	if (bytes_rxed > 0 && bytes_rxed < (int32_t) buf_size)
			rx_buf_ptr[bytes_rxed] = '\0';

	return 0;
}
