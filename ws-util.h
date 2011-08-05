/**********************************************************************
 ws-util.h - Declarations for the Winsock utility functions module.

 Source: http://tangentsoft.net/wskfaq/examples/basics/index.html
*********************************************************************/

#if !defined(WS_UTIL_H)
#define WS_UTIL_H

// Uncomment one.
//#include <winsock.h>
#include <winsock2.h>	// For MS Windows Socket Application 2.0

const char* WSAGetLastErrorMessage(const char* pcMessagePrefix,
        int nErrorID = 0);
bool ShutdownConnection(SOCKET sd, char* sock_name);
//bool	ShutdownConnection(SOCKET);
//int		init_sockets(char*[]);

//Modified functions
int		do_winsock(const char* , int , const char* , int );
int	    sendPkt(SOCKET outSoc, const char *dataPkt, int len);

#endif // !defined (WS_UTIL_H)

