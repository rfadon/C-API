#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "ws-util.h"
#include "wsa_client.h"
#include "wsa_error.h"

using namespace std;

// Comment this out to disable the shutdown-delay functionality.
//#define SHUTDOWN_DELAY
#if defined(SHUTDOWN_DELAY)
	// How long to wait after we do the echo before shutting the connection
	// down, to give the user time to start other clients, for testing 
	// multiple simultaneous connections.
	const int8_t kShutdownDelay = 1;
#endif


///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////	
uint8_t debug_mode = FALSE;
uint8_t call_mode = FALSE;
		  

///////////////////////////////////////////////////////////////////////////////
// Local Prototypes
///////////////////////////////////////////////////////////////////////////////
SOCKET setup_sock(char *sock_name, const char *sock_addr, int32_t sock_port);
SOCKET establish_connection(u_long sock_addr, u_short sock_port);


/**
 * Call functions to initialize the sockets
 * 
 * @param wsa_addr -
 * @param cmd_sock -
 * @param data_sock -
 *
 * @return
 */
int16_t wsa_start_client(const char *wsa_addr, SOCKET *cmd_sock, 
					SOCKET *data_sock, int32_t ctrl_port, int32_t data_port)
{
	//*****
	// Starting Winsock2
	//*****
	WSAData ws_data;		// create an instance of Winsock data type
	int32_t ws_err_code;	// get error code
	int16_t result = 0;		// result returned from a function

	// MAKEWORD(2, 2) - request for version 2.2 of Winsock on the system
	if ((ws_err_code = WSAStartup(MAKEWORD(2, 2), &ws_data)) != 0) {
		doutf(DHIGH, "WSAStartup() returned error code %d. ", ws_err_code);
		doutf(DHIGH, "%s\n", WSAGetLastErrorMessage(
			"Error creating an instance of Winsock in Windows!\n"));
		return WSA_ERR_WINSOCKSTARTUPFAILED;	// random # for now
	}


	//*****
	// Create command socket
	//*****
	SOCKET cmd_socket = setup_sock("WSA 'command' socket", wsa_addr, ctrl_port);
	if (cmd_socket == INVALID_SOCKET) {
		return WSA_ERR_ETHERNETCONNECTFAILED;
	}
	else {
		*cmd_sock = cmd_socket;
		printf("connected.\n");
	}


	//*****
	// Create data socket
	//*****
	SOCKET data_socket = setup_sock("WSA 'data' socket", wsa_addr, data_port);
	if (data_socket == INVALID_SOCKET) {
		result = WSA_ERR_ETHERNETCONNECTFAILED;
	}
	else {
		*data_sock = data_socket;
		printf("connected.\n");
	}

	return result;
}


/**
 * 
 *
 * @param cmd_sock -
 * @param data_sock -
 * 
 * @return 
 */
int16_t wsa_close_client(SOCKET cmd_sock, SOCKET data_sock)
{
	printf("\nClosing down this application...\n");
#if defined(SHUTDOWN_DELAY)
	// Delay for a bit, so we can start other clients.  This is strictly
	// for testing purposes, so you can convince yourself that the 
	// server is handling more than one connection at a time.
	printf("... in %d seconds: ", 
		kShutdownDelay);
	fflush(stdin);

	for (int i = 0; i < kShutdownDelay; ++i) {
		Sleep(1000);
		printf(".");
		fflush(stdin);
	}
	printf("\n");
#endif

	// Shut COMMAND socket connection down
	//fflush(stdin);
	if (ShutdownConnection(cmd_sock, "command socket"))
		printf("Command socket connection is down.\n");
	else {
		fprintf(stderr, "\nERROR: %s\n", 
			WSAGetLastErrorMessage("Shutdown 'command' socket connection"));
		WSACleanup();
		return WSA_ERR_SOCKETDROPPED;
	}


	// Shut DATA socket connection down
	//fflush(stdin);
	if (ShutdownConnection(data_sock, "data socket"))
		printf("Data socket connection is down.\n");
	else {
		fprintf(stderr, "\nERROR: %s\n", 
			WSAGetLastErrorMessage("Shutdown 'data' socket connection"));
		WSACleanup();
		return WSA_ERR_SOCKETDROPPED;
	}

	// Shut Winsock back down and take off.
	WSACleanup();

	return 0;
}


/**
 * Look up, verify and establish the socket once deemed valid
 *
 * @param sock_name - Name of the socket (ex. server, client)
 * @param sock_addr -
 * @param sock_port -
 *
 * @return Newly-connected socket when succeed, or INVALID_SOCKET when fail.
 */
SOCKET setup_sock(char *sock_name, const char *sock_addr, int32_t sock_port)
{
	// Find the server's address
	//printf("Looking up %s address: ", sock_name);
	//fflush(stdin);
	SOCKET sd;

	u_long new_sock_addr = wsa_addr_check(sock_addr);
	if (new_sock_addr == INADDR_NONE) {
		fprintf(stderr, "\nError %s\n", 
			WSAGetLastErrorMessage("lookup address"));
		//return WSA_ERR_INVIPHOSTADDRESS;
		sd = INVALID_SOCKET;
	}
	else {
		// Keep record of the socket's address & port
		in_addr socAdrIn;
		memcpy(&socAdrIn, &new_sock_addr, sizeof(u_long)); 
		printf("Connecting to WSA @ %s:%d... ", 
			inet_ntoa(socAdrIn), sock_port);

		// The htons function converts a u_short from host to TCP/IP 
		// network byte order (which is big-endian)
		sd = establish_connection(new_sock_addr, htons(sock_port));
	}

	return sd;
}


/**
 * Given an address string, determine if it's a dotted-quad IP address
 * or a domain address.  If the latter, ask DNS to resolve it.  In
 * either case, return resolved IP address.  If we fail, we return
 * INADDR_NONE.
 *
 * @param sock_addr - 
 *
 * @return Resolved IP address or INADDR_NONE when failed.
 */
uint32_t wsa_addr_check(const char *sock_addr)
{
	WSADATA wsaData;
	int iResult;
	// Initialize Winsock
	if ((iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return WSA_ERR_WINSOCKSTARTUPFAILED;
	}

	u_long new_sock_addr = inet_addr(sock_addr);
	if (new_sock_addr == INADDR_NONE) {
		// sock_addr isn't a dotted IP, so resolve it through DNS
		hostent *pHE = gethostbyname(sock_addr);
		if (pHE == NULL) {
			return INADDR_NONE;
		}
		new_sock_addr = *((u_long*)pHE->h_addr_list[0]);
	}
	
	WSACleanup();

	return new_sock_addr;
}



/**
 * Connects to a given address, on a given port, both of which must be in
 * network byte order.  Returns 
 *
 * @param sock_addr -
 * @param sock_port - 
 *
 * @return Newly-connected socket when succeed, or INVALID_SOCKET when fail.
 */
SOCKET establish_connection(u_long sock_addr, u_short sock_port)
{
	sockaddr_in remoteSocIn;
	// Create a stream socket
	// AF_UNSPEC is useless w/ winsock2
	// So first check for IPv6, if failed do IPv4

	SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd != INVALID_SOCKET) {
		remoteSocIn.sin_family = AF_INET;
		remoteSocIn.sin_addr.s_addr = sock_addr;
		remoteSocIn.sin_port = sock_port;
	
		if (connect(sd, (sockaddr*)&remoteSocIn, 
			sizeof(sockaddr_in)) == SOCKET_ERROR) {
			sd = INVALID_SOCKET;
			printf("Invalid socket!!!\n");
		}
	}

	return sd;
}



/**
 * Sends a string to the server.  
 *
 * @param out_sock -
 * @param out_str -
 * @param len -
 * 
 * @returns Number of bytes sent on success, or negative otherwise.
 */
int32_t wsa_sock_send(SOCKET out_sock, char *out_str, int32_t len)
{
	//const char *temp = (const char*) out_str;
	// Send the string to the server
	//if (out_socket
	int32_t bytes_txed = send(out_sock, out_str, len, 0);
	if (bytes_txed > 0) {
		doutf(DMED, "Sent %d bytes to server.\n", bytes_txed);
	}
	else if (bytes_txed == SOCKET_ERROR) {
		printf("Sent failed.  Winsock2 error: %ld\n", WSAGetLastError());
		return WSA_ERR_SOCKETERROR;
	}
	else {
		// Client closed connection before we could reply to
		// all the data it sent, so bomb out early.
		//printf("Peer unexpectedly dropped connection!\n");
		return WSA_ERR_SOCKETDROPPED;
	}

	return bytes_txed;
}



/**
 * Gets incoming control strings from the server socket \b buf_size bytes 
 * at a time.  It does not loop to keep checking \b buf_size of bytes are
 * received.
 *
 * @param in_sock - The socket at which the data will be received.
 * @param rx_buf_ptr - A char pointer buffer to store the incoming bytes.
 * @param buf_size - The size of the buffer in bytes.
 * @param time_out - Time out in milliseconds.
 * 
 * @return Number of bytes read
 */
int32_t wsa_sock_recv(SOCKET in_sock, char *rx_buf_ptr, uint32_t buf_size,
					  uint32_t time_out)
{
	int32_t bytes_rxed = 0;
	double seconds = floor(time_out / 1000.0);
	
	//wait x msec. timeval = {secs, microsecs}.
	timeval timer = {(long)seconds, 
					(time_out - (long) (seconds * 1000)) * 1000}; 

	// First check for read-ability to the socket
	FD_SET Reader;
	
	// FD_ZERO() clears out the fd_set called socks, so that
	//   it doesn't contain any file descriptors. 
	FD_ZERO(&Reader);

	// FD_SET() adds the file descriptor "socket" to the fd_set,
	//	so that select() will return if a connection comes in
	//	on that socket (which means you have to do accept(), etc.)
	FD_SET(in_sock, &Reader);

	// Make reading of socket non-blocking w/ time-out of x msec
	if (select(0, &Reader, NULL, NULL, &timer) == SOCKET_ERROR) {
		doutf(DMED, "winsock init select() function returned with "
			"error %d\n", WSAGetLastError());
		return WSA_ERR_SOCKETERROR;
	}
	
	// if the socket is read-able, rx packet
	if (FD_ISSET(in_sock, &Reader)) {
		// read incoming strings at a time
		bytes_rxed = recv(in_sock, rx_buf_ptr, buf_size, 0);
		if (bytes_rxed == SOCKET_ERROR) {
			doutf(DMED, "winsock recv() function returned with "
				"error %d\n", WSAGetLastError());
			return WSA_ERR_SOCKETERROR;
		}

		// Terminate the last character in cmd resp string only to 0.
		if (bytes_rxed > 0 && bytes_rxed < (int32_t) buf_size)
				rx_buf_ptr[bytes_rxed] = '\0';

		// DANGER here is that the code so far doesn't handle multiple 
		// answers in the buffer... assuming 1 answer per query so far...
		//printf("%d (%d) got: %s\n", bytes_rxed, buf_size, rx_buf_ptr);
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
 * @return Number of bytes read
 */
int32_t wsa_sock_recv_data(SOCKET in_sock, char *rx_buf_ptr, uint32_t buf_size,
					  uint32_t time_out)
{
	uint32_t bytes_rxed = 0, total_bytes = 0;
	int count = 0;
	double seconds = floor(time_out / 1000.0);
	
	//wait x msec. timeval = {secs, microsecs}.
	timeval timer = {(long)seconds, 
					(time_out - (long) (seconds * 1000)) * 1000}; 

	// First check for read-ability to the socket
	FD_SET Reader;
	
	// FD_ZERO() clears out the fd_set called socks, so that
	//   it doesn't contain any file descriptors. 
	FD_ZERO(&Reader);

	// FD_SET() adds the file descriptor "socket" to the fd_set,
	//	so that select() will return if a connection comes in
	//	on that socket (which means you have to do accept(), etc.)
	FD_SET(in_sock, &Reader);


	// Loop to get incoming command buf_size bytes at a time
	do {
		// Make reading of socket non-blocking w/ time-out of x msec
		if (select(0, &Reader, NULL, NULL, &timer) == SOCKET_ERROR) {
			doutf(DMED, "winsock init select() function returned with "
				"error %d\n", WSAGetLastError());
			break;
		}
		
		// if the socket is read-able, rx packet
		if (FD_ISSET(in_sock, &Reader)) {
			// read incoming strings at a time
			bytes_rxed = recv(in_sock, rx_buf_ptr, buf_size - total_bytes, 0);
			if (bytes_rxed > 0) {
				total_bytes += bytes_rxed;

				if (total_bytes < buf_size) {
					//rx_buf_ptr[bytes_rxed] = '\0'; 
					rx_buf_ptr += bytes_rxed;
				}
				else 
					break;
			}
		}
		doutf(DLOW, "rxed: %ld - ", bytes_rxed);
	} while (total_bytes < buf_size);
	
	return total_bytes;
}


// this one will receive & convert to words/tokens.... don't think I'll need this
int16_t wsa_sock_recv_words(SOCKET in_sock, char *rx_buf_ptr[], 
							uint32_t time_out)
{
	char *rx_buf[1]; 
		rx_buf[0] = (char*) malloc(MAX_STR_LEN * sizeof(char));  
	int16_t bytes_rxed = 0, count = 0;
	int16_t w = 0, c = 0; // word & char indexes
	double seconds = floor(time_out / 1000.0);
	
	//wait x msec. timeval = {secs, microsecs}.
	timeval timer = {(long) seconds, 
					(time_out - (long) (seconds * 1000)) * 1000}; 

	// First check for read-ability to the socket
	FD_SET Reader;
	
	/* FD_ZERO() clears out the fd_set called socks, so that
	   it doesn't contain any file descriptors. */
	FD_ZERO(&Reader);

	/* FD_SET() adds the file descriptor "socket" to the fd_set,
		so that select() will return if a connection comes in
		on that socket (which means you have to do accept(), etc. */
	FD_SET(in_sock, &Reader);

	// Loop to get incoming command 1 byte at a time
	do {
		// Make reading of socket non-blocking w/ time-out of x msec
		if (select(0, &Reader, NULL, NULL, &timer) == SOCKET_ERROR) {
			printf("init select() function returned with error %d\n", 
				WSAGetLastError());
			return WSA_ERR_SOCKETERROR;
		}
		
		// if the socket is read-able, rx packet
		if (FD_ISSET(in_sock, &Reader)) {
			count = 0; // reset count
			// read incoming strings at a time
			bytes_rxed = recv(in_sock, rx_buf[0], MAX_STR_LEN, 0);

			// Count the number of words in the received string
			while (count < bytes_rxed) {
				if (strncmp(" ", &(rx_buf[0][count]), 1) == 0 || 
					strncmp("\r", &(rx_buf[0][count]), 1) == 0 || 
					strncmp("\n", &(rx_buf[0][count]), 1) == 0) {		
					// add null termination for end of each string
					if (w > 0 && c > 0) 
						rx_buf_ptr[w-1][c] = '\0';  
					c = 0;	// reset char index
				}
				else  { 
					// increment the words count
					if (c == 0) w++;

					// copy the words over
					rx_buf_ptr[w-1][c++] = toupper(rx_buf[0][count]);
				}

				count++;
			}
		} 
		else
			bytes_rxed = 0; 
	} while(bytes_rxed > 0);

	free(rx_buf[0]);

	return w;
}

/*****
 * Get response string from client socket and compare to the in-string
 * NEED THIS? MIGHT NOT.
 * @param time_out in millisecond
 *
 * @return true is the same, else falsed
 */
uint8_t get_sock_ack(SOCKET in_sock, char *ack_str, long time_out)
{	
	// to rx response string from GUI
	char *rx_buf; 
		rx_buf = (char *) malloc(20 * sizeof(char));
	
	time_t start_time;	
	start_time = time(0);			//* time in seconds

	// Wait for client response rxed before proceeds....
	// but time out if no resp in x milliseconds
	while(wsa_sock_recv(in_sock, rx_buf, MAX_STR_LEN, 5) < 1) {
		if ((time(0) - start_time) == (time_out / 1000)) 
			break; 
	};
	
	if (strncmp(rx_buf, ack_str, 7) == 0) { //strlen(success)
		if (call_mode) printf("s");
		free(rx_buf); 
		return TRUE; 
	}
	else { 
		if (call_mode) printf("f"); 
		free(rx_buf); 
		return FALSE; 
	}
}


/**
 * Get host information based on the name given either as IP or host name 
 * format
 *
 * @param name - A char pointer to store the host name
 *
 * @return
 */
int16_t wsa_get_host_info(char *name)
{
	//-----------------------------------------
	// Declare and initialize variables
	WSADATA wsaData;
	int iResult;

	DWORD dwError;
	int i = 0;

	struct hostent *remoteHost;
	char *host_name;
	struct in_addr addr;

	char **pAlias;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return WSA_ERR_WINSOCKSTARTUPFAILED;
	}

	host_name = name;

	printf("Calling gethostbyname with %s\n", host_name);
	remoteHost = gethostbyname(host_name);
	
	if (remoteHost == NULL) {
		dwError = WSAGetLastError();
		if (dwError != 0) {
			if (dwError == WSAHOST_NOT_FOUND) {
				printf("Host not found\n");
				return -1;	// TODO
			} else if (dwError == WSANO_DATA) {
				printf("No data record found\n");
				return -1;	// TODO
			} else {
				printf("Function failed with error: %ld\n", dwError);
				return -1;	// TODO
			}
		}
	} else {
		printf("Function returned:\n");
		printf("\tOfficial name: %s\n", remoteHost->h_name);
		for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
			printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
		}
		printf("\tAddress type: ");
		switch (remoteHost->h_addrtype) {
		case AF_INET:
			printf("AF_INET\n");
			break;
		case AF_NETBIOS:
			printf("AF_NETBIOS\n");
			break;
		default:
			printf(" %d\n", remoteHost->h_addrtype);
			break;
		}
		printf("\tAddress length: %d\n", remoteHost->h_length);

		i = 0;
		if (remoteHost->h_addrtype == AF_INET)
		{
			while (remoteHost->h_addr_list[i] != 0) {
				addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
				printf("\tIP Address #%d: %s\n", i, inet_ntoa(addr));
			}
		}
		else if (remoteHost->h_addrtype == AF_NETBIOS)
		{   
			printf("NETBIOS address was returned\n");
		}   
	}

	WSACleanup();

	return 0;
}


