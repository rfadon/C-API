#include "wsa4k_cli.h"
#include "ws-util.h"

using namespace std;


// Comment this out to disable the shutdown-delay functionality.
#define SHUTDOWN_DELAY

#if defined(SHUTDOWN_DELAY)
	// How long to wait after we do the echo before shutting the connection
	// down, to give the user time to start other clients, for testing 
	// multiple simultaneous connections.
	const int kShutdownDelay = 3;
#endif


///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////
#define TIMEOUT 1000			// in milliseconds
#define MAX_STR_LEN 200
#define MAX_BUF_SIZE 20

#define HISLIP 4880

const int fileSize = 1024;
const int IQbus_size = 32;
char *start = "STARTDATA\0";
char *stop = "STOPDATA\0";

const int cmd_port = 7000;//HISLIP;
const int data_port = cmd_port;//7000;
	      

///////////////////////////////////////////////////////////////////////////////
// Local Prototypes
///////////////////////////////////////////////////////////////////////////////
int	do_winsock(const char* sock_addr);
SOCKET setup_sock( char *sock_name, const char *sock_addr, int sock_port);
u_long lookup_addr(const char *sock_addr);
SOCKET establish_connection(u_long sock_addr, u_short sock_port);
int tx_sock(SOCKET out_sock, char *out_str, int len);
int rx_sock(SOCKET in_sock, char *rx_buf_ptr[], long time_out);


/**
 * Call functions to initialize the sockets
 * 
 * @param argc - 
 * @param argv -
 *
 * @return
 */
int init_client(int argc, char *argv[])
{
	// Get host and (optionally) port from the command line
    const char *svr_addr = argv[1];

    // Starting Winsock2
    WSAData ws_data;	// create an instance of Winsock data type
	int ws_err_code;	// get error code
	int result = 0;

	// MAKEWORD(2, 2) - request for version 2.2 of Winsock on the system
    if ((ws_err_code = WSAStartup(MAKEWORD(2, 2), &ws_data)) != 0) {
		printf("WSAStartup() returned error code %d. ", ws_err_code);
		printf("%s\n", WSAGetLastErrorMessage(
			"Error creating an instance of Winsock in Windows!\n"));
        return 255;	// random # for now
    }

    // Call the main example routine.
	result = do_winsock(svr_addr);

	// todo handle the result here

    // Shut Winsock back down and take off.
    WSACleanup();
    return result;
}


/**
 * The module's driver function -- we just call other functions and
 * interpret their results.
 *
 * @param sock_addr
 */
int do_winsock(const char *sock_addr)
{
	SOCKET cmd_sock = setup_sock("server", sock_addr, cmd_port);
    if (cmd_sock == INVALID_SOCKET) {
        fprintf(stderr, "%s\n", 
			WSAGetLastErrorMessage("error connecting to the server"));
        return 3;
    }
    else {
		printf("   ...connected, socket %d.\n", cmd_sock);
		// Send start flag
		printf("Sending %s...", start);
		if (tx_sock(cmd_sock, start, strlen(start)) < 0)
			return -1;
	}

	int words_rxed = 0;
	char *rx_buf[MAX_BUF_SIZE];
	// Initialized the receive buffer
	for (int i = 0; i < MAX_BUF_SIZE; i++) 
		rx_buf[i] = (char*) malloc(MAX_STR_LEN * sizeof(char));

	words_rxed = rx_sock(cmd_sock, rx_buf, TIMEOUT);
	printf("\nRxed %d words: ", words_rxed);
	for (int i = 0; i < words_rxed; i++)
		printf("%s ", rx_buf[i]);
	printf("\n");
	
	// send stop flag
	if (tx_sock(cmd_sock, stop, strlen(stop)))
		printf("%s flag sent...\n", stop);

#if defined(SHUTDOWN_DELAY)
    // Delay for a bit, so we can start other clients.  This is strictly
    // for testing purposes, so you can convince yourself that the 
    // server is handling more than one connection at a time.
    printf("Will shut down in %d seconds... (one dot per second): ", 
		kShutdownDelay);
	fflush(stdin);

    for (int i = 0; i < kShutdownDelay; ++i) {
        Sleep(1000);
		printf(".");
		fflush(stdin);
    }
    printf("\n");
#endif

    // Shut connection down
    printf("Shutting server connection down... ");
	fflush(stdin);
    if (ShutdownConnection(cmd_sock, "server"))
        printf("Server connection is down.\n");
	else
        fprintf(stderr, "\n%s\n", 
			WSAGetLastErrorMessage("Shutdown server connection"));

    printf("All done!\n");

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
SOCKET setup_sock(char *sock_name, const char *sock_addr, int sock_port)
{
	// Find the server's address
    printf("Looking up %s address... ", sock_name);
	fflush(stdin);

    u_long new_sock_addr = lookup_addr(sock_addr);
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
u_long lookup_addr(const char *sock_addr)
{
    u_long new_sock_addr = inet_addr(sock_addr);
    if (new_sock_addr == INADDR_NONE) {
        // sock_addr isn't a dotted IP, so resolve it through DNS
        hostent *pHE = gethostbyname(sock_addr);
        if (pHE == 0) {
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
 * @param out_sock
 * @param out_str
 * 
 * @returns number of bytes send on success, or negative otherwise.
 */
int tx_sock(SOCKET out_sock, char *out_str, int len)
{
	//const char *temp = (const char*) out_str;
    // Send the string to the server
	int bytes_txed = send(out_sock, out_str, len, 0);
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
 * @param
 * @param time_out Time out in milliseconds
 * 
 * @return Number of "words" read
 */
int rx_sock(SOCKET in_sock, char *rx_buf_ptr[], long time_out)
{
	char *rx_buf[1]; 
		rx_buf[0] = (char*) malloc(MAX_STR_LEN * sizeof(char));  
	int bytes_rxed = 0, count = 0;
	int w = 0, c = 0; // word & char indexes
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
 *
 * @param time_out in millisecond
 *
 * @return true is the same, else falsed
 */
bool get_sock_ack(SOCKET in_sock, char *ack_str, long time_out)
{	
	// to rx response string from GUI
	char *rx_buf[1]; rx_buf[0] = (char*) malloc(20 * sizeof(char));
	
	time_t start_time;
	
	start_time = time(0);			//* time in seconds
	// Wait for client response rxed before proceeds....
	// but time out if no resp in x seconds
	while(rx_sock(in_sock, rx_buf, 10) < 1) {
		if((time(0) - start_time) == (time_out / 1000)) 
			break; 
	};
	
	if(strncmp(rx_buf[0], ack_str, 7) == 0) { //strlen(success)
		if(test_mode) printf("s");
		free(rx_buf[0]); 
		return TRUE; 
	}
	else { 
		if(test_mode) printf("f"); 
		free(rx_buf[0]); 
		return FALSE; 
	}
}