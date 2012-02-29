#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "wsa_client_os_specific.h"
#include "wsa_client.h"
#include "wsa_error.h"
#include "wsa_debug.h"


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
	struct addrinfo hint_ai;
	int32_t getaddrinfo_result;

	// Construct local address structure
	memset(&hint_ai, 0, sizeof(hint_ai)); //Zero out structure
	hint_ai.ai_family = AF_UNSPEC;		// Address family unspec in order to
										// return socket addresses for any 
										// address family (IPv4, IPv6, etc.)
	hint_ai.ai_socktype = SOCK_STREAM;	// For TCP type. Or use 0 for any type
	hint_ai.ai_flags = 0;
	hint_ai.ai_protocol = 0;			// to auto chose the protocol
	
	// Check the address at the given port
	getaddrinfo_result = getaddrinfo(sock_addr, sock_port, &hint_ai, &ai_list);
	if (getaddrinfo_result != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_result));
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
	struct addrinfo *ai_list = 0;
	int16_t result;

	// Check the address at the given port
	result = _addr_check(sock_addr, sock_port, ai_list);
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
	struct addrinfo *ai_list, *ai_ptr;
	struct addrinfo hint_ai;
	int32_t getaddrinfo_result;
	int32_t temp_fd = 0;
	char str[INET6_ADDRSTRLEN];
	
	printf("setting up %s socket at port %s ... ", sock_name, sock_port);

	// Construct local address structure
	memset(&hint_ai, 0, sizeof(hint_ai)); //Zero out structure
	hint_ai.ai_family = AF_UNSPEC;		// Address family unspec in order to
										// return socket addresses for any 
										// address family (IPv4, IPv6, etc.)
	hint_ai.ai_socktype = SOCK_STREAM;	// For TCP type. Or use 0 for any type
	hint_ai.ai_flags = 0;
	hint_ai.ai_protocol = 0;			// to auto chose the protocol

	// Check the address at the given port
	getaddrinfo_result = getaddrinfo(sock_addr, sock_port, &hint_ai, &ai_list);
	if (getaddrinfo_result != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_result));
		return WSA_ERR_INVIPHOSTADDRESS;
	}

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
			wsa_close_sock(temp_fd);
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
	
	inet_ntop(ai_ptr->ai_family, get_in_addr(
		(struct sockaddr *) ai_ptr->ai_addr), str, sizeof(str));
	printf("connected to %s\n", str);

	freeaddrinfo(ai_list); // all done with this list

	*sock_fd = temp_fd;
	
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
			doutf(DMED, "Sent '%s' (%d bytes) to server.\n", out_str, bytes_txed);
			
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
 * received. \n
 * This socket receive function makes used of select() function to check for
 * data availability before receiving.
 *
 * @param sock_fd - The socket at which the data will be received.
 * @param rx_buf_ptr - A char pointer buffer to store the incoming bytes.
 * @param buf_size - The size of the buffer in bytes.
 * @param time_out - Time out in milliseconds.
 * 
 * @return Number of bytes read on successful or a negative value on error
 */
int32_t wsa_sock_recv(int32_t sock_fd, char *rx_buf_ptr, int32_t buf_size,
					  uint32_t time_out)
{
	int32_t bytes_rxed = 0;
	fd_set read_fd;		// temp file descriptor for select()
	int32_t ret_val;	// return value of a function

	struct timeval timer;
	long seconds = (long) floor(time_out / 1000.0);

	// first set the time out timer
	timer.tv_sec = seconds;
	timer.tv_usec = time_out - (seconds * 1000);

	// *****
	// Use select() & FD_SET to make this receiving non-blocking (i.e. with
	// time out value)
	// *****

	// FD_ZERO() clears out the fd_set called socks, so that
	//   it doesn't contain any file descriptors.
	FD_ZERO(&read_fd);

	// FD_SET() adds the file descriptor "socket" to the fd_set,
	//	so that select() will return if a connection comes in
	//	on that socket (which means you have to do accept(), etc.)
	FD_SET(sock_fd, &read_fd);

	ret_val = select(sock_fd + 1, &read_fd, NULL, NULL, &timer);
	// Make reading of socket non-blocking w/ time-out of s.ms sec
	if (ret_val == -1) {
		doutf(DMED, "init select() function returned with error");
		perror("select()");
		return WSA_ERR_SOCKETERROR;
	}
	else if (ret_val) {
		doutf(DMED, "Data is available now.\n");
	}
	else {
		printf("No data received within %d milliseconds.\n", time_out);
		return WSA_ERR_QUERYNORESP;
	}

	// if the socket is read-able, rx packet
	if (FD_ISSET(sock_fd, &read_fd)) {
		// read incoming strings at a time
		bytes_rxed = recv(sock_fd, rx_buf_ptr, buf_size, 0);
		
		// checked the return value
		if (bytes_rxed <= 0) {
			if (bytes_rxed == 0) {
				// Connection closed
				doutf(DMED, "Connection is already closed.\n");
				return WSA_ERR_SOCKETERROR;
			}
			else {
				perror("recv");
				return WSA_ERR_SOCKETSETFUPFAILED;
			}
		}

		// Terminate the last character in cmd resp string only to 0
		if (bytes_rxed > 0 && bytes_rxed < (int32_t) buf_size)
			rx_buf_ptr[bytes_rxed] = '\0';
		
		doutf(DMED, "Received (%d bytes): %s\n\n", bytes_rxed, rx_buf_ptr);
	}
		
	return bytes_rxed;
}


/**
 * Gets incoming data packet from the server socket \b buf_size bytes 
 * at a time.  This function will check to ensure the \b buf_size of bytes
 * are received.
 *
 * @param in_sock - The socket at which the data will be received.
 * @param rx_buf_ptr - A char pointer buffer to store the incoming bytes.
 * @param buf_size - The size of the buffer in bytes.
 * @param time_out - Time out in milliseconds.
 * 
 * @return Number of bytes read on successful or a negative value on error
 */
int32_t wsa_sock_recv_data(int32_t sock_fd, char *rx_buf_ptr, 
						   int32_t buf_size, uint32_t time_out)
{
	int32_t bytes_rxed = 0;
	int32_t total_bytes = 0;
	int32_t bytes_expected = buf_size;
	uint16_t retry = 1;

	do {
		bytes_rxed = wsa_sock_recv(sock_fd, rx_buf_ptr, bytes_expected, time_out);
		if (bytes_rxed > 0) {
			retry = 0;
			total_bytes += bytes_rxed;

			if (total_bytes < buf_size) {
				//rx_buf_ptr[bytes_rxed] = '\0'; 
				rx_buf_ptr += bytes_rxed;
				bytes_expected -= bytes_rxed;
			}
			else {
				doutf(DLOW, "total rxed: %ld - ", total_bytes);
				break;
			}
			doutf(DLOW, "rxed: %ld - ", bytes_rxed);
		}
		else {
			// if got error, try again to make sure?
			if (retry == 2)
				return bytes_rxed;
			retry++;
		}
	} while (1);

	return total_bytes;
}


// TODO: get_sock_ack() ?
//		 wsa_get_host_info() ?
