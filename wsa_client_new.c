
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "wsa_client.h"


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
 * Given a client address string, determine if it's a dotted-quad IP address
 * or a domain address.  If the latter, ask DNS to resolve it.  In
 * either case, return resolved IP address.  If we fail, we return
 * INADDR_NONE.
 *
 * @param sock_addr - 
 *
 * @return 0 upon successful, or a negative value when failed.
 */
int16_t wsa_addr_check(const char *sock_addr, const char *port, 
						struct addrinfo *ai_list)//int *sock_fd)
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
	result = getaddrinfo(sock_addr, port, &hint_ai, &ai_list);
	if (result != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
		return WSA_ERR_INVIPHOSTADDRESS;
	}

	return 0;
}

/**
 * Look up, verify and establish the socket once deemed valid
 *
 * @param sock_name - Name of the socket (ex. server, client)
 * @param sock_addr -
 * @param port -
 * @param sock_fd -
 *
 * @return Newly-connected socket when succeed, or INVALID_SOCKET when fail.
 */
int16_t setup_sock(char *sock_name, const char *sock_addr, const char *port,
				   int *sock_fd)
{
	printf("setting up %s socket at port %s ... ", sock_name, port);

	struct addrinfo *ai_list, *ai_ptr;
	int16_t result;	
	int yes = 1;
	int temp_fd;

	result = wsa_addr_check(sock_addr, port, ai_list);
	if (result < 0)
		return result;

	// loop through all the results and bind to the first we can
	for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
		// Check to find a valid socket
		temp_fd = socket(ai_ptr->ai_family, ai_ptr->ai_socktype,
			ai_ptr->ai_protocol);
		if (temp_fd == -1) {
			perror("server: socket() error");
			continue;
		}

		// establish the client connection
		if (connect(temp_fd, ai_ptr->ai_addr, ai_ptr->ai_addrlen) == -1) {
			close(temp_fd);
			perror("client: connect");
			continue;
		}

		break; // successfully connected if got here
	}

	// If no address succeeded
	if (ai_ptr == NULL)  {
		fprintf(stderr, "client: failed to connect\n");
		return WSA_ERR_ETHERNETCONNECTFAILED;
	}
	else printf("binded.\n");

	freeaddrinfo(ai_list); // all done with this list

	*sock_fd = temp_fd;
	
	return 0;
}


int16_t wsa_start_client(const char *wsa_addr, int *ctrl_sock_fd, 
				int *data_sock_fd, char *ctrl_port, char *data_port)
{
	
	int16_t result = 0;		// result returned from a function

	//*****
	// Create command socket
	//*****

	//*****
	// Create data socket
	//*****

	return 0;
}