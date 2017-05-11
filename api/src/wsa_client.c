#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <errno.h>
#include <math.h>
#include "wsa_client_os_specific.h"
#include "wsa_client.h"
#include "wsa_error.h"
#include "wsa_debug.h"

#if defined(_WIN32) && defined(UNICODE)
# undef gai_strerror
# define gai_strerror gai_strerrorA
#endif


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
		return &(((struct sockaddr_in *) sock_addr)->sin_addr);
	}

	return &(((struct sockaddr_in6 *) sock_addr)->sin6_addr);
}


/**
 * Local function that does address check with all the proper socket setup and
 * call getaddrinfo() to verify.
 *
 * @param sock_addr - a const char pointer, storing the IP address
 * @param sock_port - a const char pointer, storing the socket port
 * @param ai_list - a pointer to addrinfo structure, storing the desired 
 *		socket setup
 *
 * @return 0 upon successful, or a negative value when failed.
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
		doutf(DHIGH, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_result));
		return WSA_ERR_INVIPHOSTADDRESS;
	}
	
	return 0;
}

/**
 * Given a client address string and the port number, determine if it's 
 * a dotted-quad IP address or a domain address.
 *
 * @param sock_addr - a const char pointer, storing the IP address
 * @param sock_port - a const char pointer, storing the socket port
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
		doutf(DHIGH, "getaddrinfo: %s\n", gai_strerror(result));
		return WSA_ERR_INVIPHOSTADDRESS;
	}

	freeaddrinfo(ai_list);
	
	return 0;
}


/**
 * similar to inet_ntop so that windows XP could use it
 */
const char *_inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
{
	if (af == AF_INET) {
		struct sockaddr_in in;

		memset(&in, 0, sizeof(in));
		in.sin_family = AF_INET;
		memcpy(&in.sin_addr, src, sizeof(struct in_addr));
		getnameinfo((struct sockaddr *) &in, sizeof(struct
			sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);

		return dst;
	}
	else if (af == AF_INET6) {
		struct sockaddr_in6 in;

		memset(&in, 0, sizeof(in));
		in.sin6_family = AF_INET6;
		memcpy(&in.sin6_addr, src, sizeof(struct in6_addr));
		getnameinfo((struct sockaddr *) &in, sizeof(struct
			sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);

		return dst;
	}
	return NULL;	
}

/**
 * Look up, verify and establish the socket once deemed valid
 *
 * @param sock_name - Name of the socket (ex. server, client)
 * @param sock_addr - A const char pointer, storing the IP address
 * @param sock_fd - A int32_t pointer, storing specific socket value to be set up
 * @param sock_port - A const char pointer, storing the socket port
 *
 * @return Newly-connected socket when succeed, or INVALID_SOCKET when fail.
 */
int16_t wsa_setup_sock(char *sock_name, const char *sock_addr, 
					   int32_t *sock_fd, const char *sock_port, int16_t timeout)
{
	struct addrinfo *ai_list, *ai_ptr;
	struct addrinfo hint_ai;
	int32_t getaddrinfo_result;
	int32_t temp_fd = 0;
	char str[INET6_ADDRSTRLEN];
	int32_t result;
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
		doutf(DHIGH, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_result));
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
#ifdef _WIN32
        result = setsockopt(temp_fd, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec  = timeout / 1000;
        tv.tv_usec = timeout * 1000;

        /* Ignore result */ setsockopt(temp_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
#endif

        // establish the client connection
        if (connect(temp_fd, ai_ptr->ai_addr, (int)ai_ptr->ai_addrlen) == -1) {
            wsa_close_sock(temp_fd);
            perror("client: connect() error");
            continue;
        }

        break; // successfully connected if got to here
    }

	// If no address succeeded
	if (ai_ptr == NULL)  {
		doutf(DHIGH, "client: failed to connect\n");
		return WSA_ERR_ETHERNETCONNECTFAILED;
	}
	
	_inet_ntop(ai_ptr->ai_family, get_in_addr(
		(struct sockaddr *) ai_ptr->ai_addr), str, sizeof(str));

	freeaddrinfo(ai_list); // all done with this list

	*sock_fd = temp_fd;
	
	return 0;
}


/**
 * Sends a string to the server.  
 *
 * @param sock_fd - The socket at which the data will be received.
 * @param out_str - A pointer to the string to be sent out
 * @param len - An int32_t value, storing the lenght of the string to be sent
 * 
 * @returns Number of bytes sent on success, or negative otherwise.
 */
int32_t wsa_sock_send(int32_t sock_fd, char const *out_str, int32_t len)
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
			doutf(DLOW, "Sent '%s' (%d bytes) to server.\n", out_str, bytes_txed);
			
			// update all the count
			total_txed += bytes_txed;
			bytes_left -= bytes_txed;
		} else if (bytes_txed == -1) {
			return WSA_ERR_SOCKETERROR;
        } else {
			// Client closed connection before we could reply to
			// all the data it sent, so bomb out early.
			return WSA_ERR_SOCKETDROPPED;
		}
	}

	return total_txed;
}


/**
 * Reads data from the given server socket \b buf_size bytes 
 * at a time.  It does not loop to keep checking \b buf_size of bytes are
 * received. \n
 * This socket receive function makes used of select() function to check for
 * data availability before receiving.
 *
 * @param sock_fd - The socket at which the data will be received.
 * @param rx_buf_ptr - A uint8 pointer buffer to store the incoming bytes.
 * @param buf_size - The size of the buffer in bytes.
 * @param time_out - Time out in milliseconds.
 * @param bytes_received - Pointer to int32_t storing number of bytes read (on success)
 * 
 * @return 0 on success or a negative value on error
 */		
int16_t wsa_sock_recv(int32_t sock_fd, uint8_t *rx_buf_ptr, int32_t buf_size,
					  uint32_t time_out, int32_t *bytes_received)
{
	fd_set read_fd;		// temp file descriptor for select()
	int32_t ret_val;	// return value of a function

	struct timeval timer;
	long seconds = (long) floor(time_out / 1000.0);

	// first set the time out timer
	timer.tv_sec = seconds;
	timer.tv_usec = (long) (time_out - (seconds * 1000)) * 1000;
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
		doutf(DHIGH, "init select() function returned with error %d (\"%s\")", errno, strerror(errno));

		return WSA_ERR_SOCKETERROR;
	}
	else if (ret_val) {
		doutf(DLOW, "Data is available now.\n");
	}
	else {
		doutf(DLOW, "No data received within %d milliseconds.\n", time_out);
        if(ret_val) {
    		doutf(DMED, "In wsa_sock_recv: select returned %d\n", ret_val);
        }

		return WSA_ERR_QUERYNORESP;
	}

	// if the socket is read-able, rx packet
	if (FD_ISSET(sock_fd, &read_fd)) {
		// read incoming data buf_size at a time
		// Need to cast the buffer pointer to char*
		// since that is the data type on Windows
		ret_val = recv(sock_fd, (char *) rx_buf_ptr, buf_size, 0);
		
		// checked the return value
		if (ret_val == 0) {
			// Connection closed
			doutf(DMED, "Connection is already closed.\n");

			return WSA_ERR_SOCKETERROR;
		}
		else if (ret_val < 0) {
			doutf(DHIGH, "recv() function returned with error %d (\"%s\")", errno, strerror(errno));

			return WSA_ERR_SOCKETSETFUPFAILED;
		}
		
		doutf(DLOW, "Received (%d bytes)\n\n", ret_val);
	}
		
	*bytes_received = ret_val;
	return 0;
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
 * @param total_bytes - Pointer to int32_t storing number of bytes read (on success)
 * 
 * @return 0 on success or a negative value on error
 */
int16_t wsa_sock_recv_data(int32_t sock_fd, uint8_t *rx_buf_ptr, 
						   int32_t buf_size, uint32_t time_out, int32_t *total_bytes)
{
	int16_t recv_result = 0;
	int32_t bytes_received = 0;
	int32_t bytes_expected = buf_size;
	uint16_t retry = 0;
	uint16_t try_limit = 3;

	*total_bytes = 0;
	
	do {

		recv_result = wsa_sock_recv(sock_fd, rx_buf_ptr, bytes_expected, time_out / (uint32_t) try_limit, &bytes_received);
		if (recv_result == 0) {
			retry = 0;
			*total_bytes += bytes_received;

			if (*total_bytes < buf_size) {
				rx_buf_ptr += bytes_received;
				bytes_expected -= bytes_received;
			}
			else {
				doutf(DLOW, "total bytes received: %d - ", *total_bytes);
				break;
			}

			doutf(DLOW, "bytes received: %d - ", bytes_received);
		}
		else {
			// if got error, try again to make sure?
			if (retry == (try_limit - 1))
				return recv_result;
			retry++;
		}
	} while (1);

	return 0;
}

