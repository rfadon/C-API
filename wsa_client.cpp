#include "wsa_client.h"
//#include "ws-util.h"

using namespace std;

// Comment this out to disable the shutdown-delay functionality.
#define SHUTDOWN_DELAY
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
uint8_t test_mode = FALSE;

//const int32_t fileSize = 1024;
//const int32_t IQbus_size = 32;
char *start = "STARTDATA\0";
char *stop = "STOPDATA\0";

const int32_t cmd_port = 7000;// swap this w/ HISLIP
const int32_t data_port = 7000;//;
	      

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
int16_t wsa_start_client(const char *wsa_addr, SOCKET *cmd_sock, SOCKET *data_sock)
{
	//*****
    // Starting Winsock2
	//*****
    WSAData ws_data;		// create an instance of Winsock data type
	int32_t ws_err_code;	// get error code
	int16_t result = 0;		// result returned from a function

	// MAKEWORD(2, 2) - request for version 2.2 of Winsock on the system
    if ((ws_err_code = WSAStartup(MAKEWORD(2, 2), &ws_data)) != 0) {
		printf("WSAStartup() returned error code %d. ", ws_err_code);
		printf("%s\n", WSAGetLastErrorMessage(
			"Error creating an instance of Winsock in Windows!\n"));
        return 255;	// random # for now
    }


	//*****
	// Create command socket
	//*****
	SOCKET cmd_socket = setup_sock("WSA 'command' socket", wsa_addr, cmd_port);
    if (cmd_socket == INVALID_SOCKET) {
        fprintf(stderr, "%s\n", 
			WSAGetLastErrorMessage("error connecting to the server"));
        return 3;
    }
    else {
		*cmd_sock = cmd_socket;
		printf("   ...connected, socket %d.\n", cmd_socket);
	}


	//*****
	// Create data socket
	//*****
	// TODO: add data socket
/*	SOCKET data_socket = setup_sock("WSA 'data' socket", wsa_addr, data_port);
    if (data_socket == INVALID_SOCKET) {
        fprintf(stderr, "%s\n", 
			WSAGetLastErrorMessage("error connecting to the server"));
        result = -1;
    }
    else {
		*data_sock = data_socket;
		// TODO: remove this section
		printf("   ...connected, socket %d.\n", data_socket);
	}*/

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
#if defined(SHUTDOWN_DELAY)
    // Delay for a bit, so we can start other clients.  This is strictly
    // for testing purposes, so you can convince yourself that the 
    // server is handling more than one connection at a time.
    printf("\nWill shut down sockets in %d seconds... (one dot per second): ", 
		kShutdownDelay);
	fflush(stdin);

    for (int i = 0; i < kShutdownDelay; ++i) {
        Sleep(1000);
		printf(".");
		fflush(stdin);
    }
#endif

    // Shut COMMAND socket connection down
	//fflush(stdin);
    if (ShutdownConnection(cmd_sock, "command socket"))
        printf("Command socket connection is down.\n");
	else
		fprintf(stderr, "\nERROR: %s\n", 
			WSAGetLastErrorMessage("Shutdown 'command' socket connection"));

/*
	// Shut DATA socket connection down
	//fflush(stdin);
    if (ShutdownConnection(data_sock, "data socket"))
        printf("Command socket connection is down.\n");
	else
		fprintf(stderr, "\nERROR: %s\n", 
			WSAGetLastErrorMessage("Shutdown 'data' socket connection"));
*/
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
    printf("Looking up %s address... ", sock_name);
	fflush(stdin);

    u_long new_sock_addr = wsa_verify_addr(sock_addr);
    if (new_sock_addr == INADDR_NONE) {
        fprintf(stderr, "\nError %s\n", 
			WSAGetLastErrorMessage("lookup address"));
        return 3;
    }

	// Keep record of the socket's address & port
	in_addr socAdrIn;
    memcpy(&socAdrIn, &new_sock_addr, sizeof(u_long)); 
	printf("%s:%d\n", inet_ntoa(socAdrIn), sock_port); 

    printf("Connecting to %s ...", sock_name);
	//fflush(stdin);

	// The htons function converts a u_short from host to TCP/IP 
	// network byte order (which is big-endian)
    SOCKET sd = establish_connection(new_sock_addr, htons(sock_port));
	if (sd == INVALID_SOCKET) 
		printf("%s socket setup failed!\n", sock_name);

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
u_long wsa_verify_addr(const char *sock_addr)
{
	WSADATA wsaData;
	int iResult;
	// Initialize Winsock
    if ((iResult = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return -1;
    }

	u_long new_sock_addr = inet_addr(sock_addr);
    if (new_sock_addr == INADDR_NONE) {
        // sock_addr isn't a dotted IP, so resolve it through DNS
        hostent *pHE = gethostbyname(sock_addr);//wsa_get_host_info
        if (pHE == NULL) {
            return INADDR_NONE;
        }
        new_sock_addr = *((u_long*)pHE->h_addr_list[0]);
    }

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
    // Create a stream socket
	// Unspecified to do both IPv4 & 6. TCP w/ no specific protocol
	// TODO: future consideration: first try with AF_INET6, when fail
	// try the below.  AF_UNSPEC is useless w/ winsock2
	SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd != INVALID_SOCKET) {
        sockaddr_in remoteSocIn;
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
int16_t wsa_sock_send(SOCKET out_sock, char *out_str, int32_t len)
{
	//const char *temp = (const char*) out_str;
    // Send the string to the server
	int16_t bytes_txed = send(out_sock, out_str, len, 0);
    if (bytes_txed > 0) {
        printf("Sent %d bytes to server.\n", bytes_txed);
    }
    else if (bytes_txed == SOCKET_ERROR) {
        printf("Sent failed.  Socket error/closed! Error: %ld\n", 
			WSAGetLastError());
		return -1;
    }
    else {
        // Client closed connection before we could reply to
        // all the data it sent, so bomb out early.
        printf("Peer unexpectedly dropped connection!\n");
        return -1;
    }

	return bytes_txed;
}



/**
 * Gets incoming strings from the server socket ? bytes at a time
 *
 * @param in_sock -
 * @param rx_buf_ptr -
 * @param time_out - Time out in milliseconds
 * 
 * @return Number of "words" read
 */
int64_t wsa_sock_recv(SOCKET in_sock, char *rx_buf_ptr, uint32_t time_out)
{
	//char *rx_buf; 
		//rx_buf[0] = (char*) malloc(MAX_STR_LEN * sizeof(char));  
	int64_t bytes_rxed = 0;
	double seconds = floor(time_out / 1000.0);
	
	//wait x msec. timeval = {secs, microsecs}.
	timeval timer = {seconds, (time_out - (long) seconds * 1000) * 1000}; 

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
			return 0;
		}
		
		// if the socket is read-able, rx packet
		if (FD_ISSET(in_sock, &Reader)) {
			// read incoming strings at a time
			bytes_rxed = recv(in_sock, rx_buf_ptr, MAX_STR_LEN, 0);
		} 
	} while(bytes_rxed < 0);
	rx_buf_ptr[bytes_rxed] = '\0';

	return bytes_rxed;
}


// this one will receive & convert to words/tokens.... don't think I'll need this
int16_t wsa_sock_recv_words(SOCKET in_sock, char *rx_buf_ptr[], uint32_t time_out)
{
	char *rx_buf[1]; 
		rx_buf[0] = (char*) malloc(MAX_STR_LEN * sizeof(char));  
	int16_t bytes_rxed = 0, count = 0;
	int16_t w = 0, c = 0; // word & char indexes
	double seconds = floor(time_out / 1000.0);
	
	//wait x msec. timeval = {secs, microsecs}.
	timeval timer = {seconds, (time_out - (long) seconds * 1000) * 1000}; 

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
			return 0;
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
					if(w > 0 && c > 0) 
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
	// but time out if no resp in x seconds
	while(wsa_sock_recv(in_sock, rx_buf, 10) < 1) {
		if((time(0) - start_time) == (time_out / 1000)) 
			break; 
	};
	
	if(strncmp(rx_buf, ack_str, 7) == 0) { //strlen(success)
		if(test_mode) printf("s");
		free(rx_buf); 
		return TRUE; 
	}
	else { 
		if(test_mode) printf("f"); 
		free(rx_buf); 
		return FALSE; 
	}
}

//* @param net_type -
/**
 * Get host information based on the name given either as IP or host name 
 * format
 *
 * @param name -
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
        return -1;
    }

    host_name = name;

    printf("Calling gethostbyname with %s\n", host_name);
    remoteHost = gethostbyname(host_name);
    
    if (remoteHost == NULL) {
        dwError = WSAGetLastError();
        if (dwError != 0) {
            if (dwError == WSAHOST_NOT_FOUND) {
                printf("Host not found\n");
                return -1;
            } else if (dwError == WSANO_DATA) {
                printf("No data record found\n");
                return -1;
            } else {
                printf("Function failed with error: %ld\n", dwError);
                return -1;
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

    return 0;
}


/**
 * Print a list of host names and the associated IP available to a user's PC.
 *
 * @param ip_list -
 *
 * @return Number of IP addresses available.
 */
int16_t wsa_list_ips(char **ip_list) 
{
	// TODO: detect all the IPs available to the PC...
	// Better yet... get only IP of WSA by using a specified name.
	// Can only verify w/in a user's ntwk using subnet mask...
	
	//TODO modify wsa_get_host_info() to find out if the given IP is 
	// IPv4 or 6 & pass that for when setup AF_NET info...
	// & also if ip address is given instead.

	// TODO use gethostname() to list <host name> (<IP>)

	ip_list[0] = "192.168.215.107";
	printf("\t1. %s\n", ip_list[0]);

	return 1;
}
