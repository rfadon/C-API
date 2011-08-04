
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

////////////////////////////////////////////////////////////////////////
// Constants
const int fileSize	= 1024;
const int IQbus_size	= 32;
const char* start	= "STARTDATA";
const char* stop	= "STOPDATA";
const int startLen	= strlen(start);

const int defaultServerPort = 7000;//4242;
const int defaultClientPort = defaultServerPort;//7001;
	      

////////////////////////////////////////////////////////////////////////
// Prototypes
SOCKET setup_sock( char* sock_name, const char* sock_addr, int sock_port);
u_long lookup_addr(const char* svr_addr);
SOCKET establish_connection(u_long svr_addr, u_short svr_port);
bool tx_sock(SOCKET sd, const char* pkt_msg);
int rx_sock(SOCKET sd);


/***
 * Call functions to initialize the sockets
 * @param:
 *
 ***/
int init_client(int argc, char* argv[])
{
	// Get host and (optionally) port from the command line
    const char* svr_addr = argv[1];
    int svr_port = defaultServerPort;//atoi(argv[2]);
	const char* clt_addr = argv[2];
	int clt_port = defaultClientPort;
	if(argc > 3)
		clt_port = atoi(argv[3]);

    // Starting Winsock2
    WSAData ws_data;	// create an instance of Winsock data type
	int ws_err_code;				// get error code

	// MAKEWORD(2, 2) - request for version 2.2 of Winsock on the system
    if ((ws_err_code = WSAStartup(MAKEWORD(2, 2), &ws_data)) != 0) {
		printf("WSAStartup() returned error code %d. ", ws_err_code);
		printf("%s\n", WSAGetLastErrorMessage(
			"Error creating an instance of Winsock in Windows!\n"));
        return 255;	// random # for now
    }

    // Call the main example routine.
	int retval = do_winsock(svr_addr, svr_port, clt_addr, clt_port);

    // Shut Winsock back down and take off.
    WSACleanup();
    return 0;
}


//// do_winsock /////////////////////////////////////////////////////////
// The module's driver function -- we just call other functions and
// interpret their results.

int do_winsock(const char* svr_addr, int svr_port, const char* clt_addr, 
			   int clt_port)
{
	SOCKET svr_sock = setup_sock("server", svr_addr, svr_port);
    if (svr_sock == INVALID_SOCKET) {
        fprintf(stderr, "%s\n", 
			WSAGetLastErrorMessage("error connecting to the server"));
        return 3;
    }
    else {
		printf("   ...connected, socket %d.\n", svr_sock);
		// Send start flag
		if(tx_sock(svr_sock, start))
			printf("STARTDATA flag sent...\n");
		else {
			printf("STARTDATA flag sending failed... exit connection\n");
			return 3;
		}
	}

	int bytesRxed = rx_sock(svr_sock);
	printf("\ngot %d\n", bytesRxed);

	// Receive incoming msg
	//printf("Receiving data from remote server/host..." << 
	//			inet_ntoa(svr_addrIn)
	//		 << ": " << svr_port << endl;
/*	SOCKET clientSoc = setup_sock("client", clt_addr, clt_port);
	if (clientSoc == INVALID_SOCKET) {
        fprintf(stderr, "%s\n", 
			WSAGetLastErrorMessage("error connecting to the client socket"));
        return 3;
    }
    else {
		printf("   ...connected, socket %s.\n", clientSoc);

		int bytesRxed = rx_sock(clientSoc);
		if (bytesRxed <= 0) {
			fprintf(stderr, 
				"\nServer/Client unexpectedly closed the connection.\n");
		}
		else {	// Stop the server from sending more data...
			if (bytesRxed < fileSize) 
				printf("Only %d received for the file...\n", bytesRxed);
			else if (bytesRxed > fileSize)
				fprintf(stderr, "FYI, likely data overflow.\n");

			if (tx_sock(svr_sock, stop))
				printf("STOPDATA flag sent...\n");
			else {
				printf("STOPDATA flag sending failed... exit connection\n");
				return 3;
			}
		}
	}

*/
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
    if (ShutdownConnection(svr_sock, "server"))
        printf("Server connection is down.\n");
	else
        fprintf(stderr, "\n%s\n", 
			WSAGetLastErrorMessage("Shutdown server connection"));
/*
	if (ShutdownConnection(clientSoc, "client"))
        printf("Client connection is down.\n");
    else
        fprintf(stderr, "\n%s\n", 
			WSAGetLastErrorMessage("Shutdown client connection"));
*/
    printf("All done!\n");

    return 0;
}


SOCKET setup_sock( char* sock_name, const char* sock_addr, int sock_port)
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

//// lookup_addr /////////////////////////////////////////////////////
// Given an address string, determine if it's a dotted-quad IP address
// or a domain address.  If the latter, ask DNS to resolve it.  In
// either case, return resolved IP address.  If we fail, we return
// INADDR_NONE.
u_long lookup_addr(const char* svr_addr)
{
    u_long new_svr_addr = inet_addr(svr_addr);
    if (new_svr_addr == INADDR_NONE) {
        // svr_addr isn't a dotted IP, so resolve it through DNS
        hostent* pHE = gethostbyname(svr_addr);
        if (pHE == 0) {
            return INADDR_NONE;
        }
        new_svr_addr = *((u_long*)pHE->h_addr_list[0]);
    }

    return new_svr_addr;
}


//// establish_connection ///////////////////////////////////////////////
// Connects to a given address, on a given port, both of which must be
// in network byte order.  Returns newly-connected socket if we succeed,
// or INVALID_SOCKET if we fail.
SOCKET establish_connection(u_long svr_addr, u_short svr_port)
{
    // Create a stream socket
	// Unspecified to do both ipv4 & 6. TCP w/ no specific protocol
	// Hmmmmm..... stuck w/ a IPv4 version
	SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
    //SOCKET sd = socket(AF_UNSPEC, SOCK_STREAM, 6);
    if (sd != INVALID_SOCKET) {
        sockaddr_in remoteSocIn;
        remoteSocIn.sin_family = AF_INET;//AF_UNSPEC;//
        remoteSocIn.sin_addr.s_addr = svr_addr;
        remoteSocIn.sin_port = svr_port;

        if (connect(sd, (sockaddr*)&remoteSocIn, 
			sizeof(sockaddr_in)) == SOCKET_ERROR) {
            sd = INVALID_SOCKET;
			printf("Invalid socket!!!\n");
        }
    }

    return sd;
}


//// tx_sock //////////////////////////////////////////////////////////
// Sends the echo packet to the server.  Returns true on success,
// false otherwise.
bool tx_sock(SOCKET sd, const char* pkt_msg)
{
    // Send the string to the server
	int nTemp = send(sd, pkt_msg, strlen(pkt_msg), 0);
    if (nTemp > 0) {
        printf("Sent %d bytes to server.\n", nTemp);
    }
    else if (nTemp == SOCKET_ERROR) {
        return false;
    }
    else {
        // Client closed connection before we could reply to
        // all the data it sent, so bomb out early.
        printf("Peer unexpectedly dropped connection!\n");
        return false;
    }
	return true;
}


//// rx_sock /////////////////////////////////////////////////////////
// Receive the incoming packet and check it for sanity.  Returns -1 on 
// error, 0 on connection closed, > 0 on success.
int rx_sock(SOCKET sd)
{
    // Read reply from server
    char acReadBuffer[IQbus_size];
    int nTotalBytes = 0;
	int nNewBytes = 0;

	//fstream IQ_fptr;
	//IQ_fptr.open("..\\IQ_data.csv", ios::out); // saved file to where the code is
	//IQ_fptr.open("IQ_data.csv", ios::out);
	//if(IQ_fptr.is_open() == 0) printf("Error: File was not open.");

	printf("\nReceiving data from remote server/host...\n");

	// use this loop to modify the length of your data file
    while (nTotalBytes < fileSize) {
		printf("Looping to rx 32bit data trunk... \n");
		//nNewBytes = recv(sd, acReadBuffer, IQbus_size, 0);
		do {
			printf("waiting for data...\n");;
			nNewBytes = recv(sd, acReadBuffer, IQbus_size, 0);
			if (nNewBytes > 0)
				printf(" received %d bytes.\n", nNewBytes);
			else if (nNewBytes == 0)
				printf("Connection closed!\n");
			else
				printf("recv() failed. Error %d\n", WSAGetLastError());			
		} while (nNewBytes>0);

		if(acReadBuffer[0] == NULL || acReadBuffer[0] == '\n') 
		{
			printf("Received only %d bytes. Txn terminated. Closing file!",
				nTotalBytes);
			return nTotalBytes;
		}
		else 
			printf("...data received: %s\n", acReadBuffer);		

        if (nNewBytes == SOCKET_ERROR) {
            return -1;
        }
        else if (nNewBytes == 0) {
            fprintf(stderr, "Connection closed by peer.\n");
            break;
        }
		//printf("\tnNewBytes: " << nNewBytes << endl;
        nTotalBytes += nNewBytes;
    }
	
	printf("Received packet is: %s\n", acReadBuffer);

	//IQ_fptr.close();

    return nTotalBytes;
}