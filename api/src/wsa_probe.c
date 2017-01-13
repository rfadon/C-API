
#include <stdio.h>
#include <stdlib.h>

#ifndef STANDALONE
#include "wsa_client_os_specific.h"
#include "wsa_client.h"
#include "wsa_api.h"
#include "wsa_error.h"
#include "wsa_debug.h"
#else
# ifdef _WIN32
#  include <windows.h>
#  pragma comment(lib, "WINMM.lib")
# endif
# define doutf(x, s, ...) if(x) fprintf(stderr, s, __VA_ARGS__);
# define DLOW  0
# define DHIGH 1
# define DMED  1
#endif

#ifdef _WIN32
# include <iphlpapi.h>
# pragma comment(lib, "Ws2_32.lib")
# pragma comment(lib, "IPHLPAPI.lib")
#else
# include <string.h>
# include <unistd.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# ifndef __ANDROID__
#  include <ifaddrs.h>
# else
#  include <stdlib.h>
# endif
# include <netdb.h>
# include <errno.h>
#endif


#ifndef _WIN32
# define closesocket        close
# define WSAGetLastError()  errno
# define ioctlsocket        ioctl
# define SD_BOTH            SHUT_RDWR
# define SOCKADDR           struct sockaddr
# define SOCKET             int
#else
# define ETIMEDOUT          WSAETIMEDOUT
# define socklen_t          int
#endif

#ifndef arraysize
# define arraysize(x) (sizeof(x)/(sizeof(x[0])))
#endif


typedef struct {
#ifdef _WIN32
  IP_ADAPTER_INFO * padaptorinfo;
  IP_ADAPTER_INFO * padaptor;
  ULONG             adaptorinfosize;
#elif defined(__ANDROID__)
  int               padaptorinfo;
  int               padaptor;
#else
  struct ifaddrs *  padaptorinfo;
  struct ifaddrs *  padaptor;
  int               result;
#endif

#ifdef _WIN32
  SOCKET  socket[32];
#else
  int     socket[32];
#endif
  int socketcount;
} wsa_probe_handle;

void * wsa_probe_begin(void)
{
  int ipaddr;
  int ipmask;

  wsa_probe_handle * probe = (wsa_probe_handle*)malloc(sizeof(wsa_probe_handle));
  if(!probe) {
    return 0;
  }

  memset(probe, 0, sizeof(wsa_probe_handle));

#ifdef _WIN32
  probe->adaptorinfosize = sizeof(IP_ADAPTER_INFO);
  probe->padaptorinfo    = (IP_ADAPTER_INFO*)malloc(probe->adaptorinfosize);

  if(GetAdaptersInfo(probe->padaptorinfo, &probe->adaptorinfosize) == ERROR_BUFFER_OVERFLOW) {
    free(probe->padaptorinfo);
    probe->padaptorinfo = (IP_ADAPTER_INFO*)malloc(probe->adaptorinfosize);
    if(probe->padaptorinfo == 0) {
      free(probe);
      return 0;
    }
  }

  if(GetAdaptersInfo(probe->padaptorinfo, &probe->adaptorinfosize) != NO_ERROR) {
    free(probe->padaptorinfo);
    free(probe);
    return 0;
  }
#elif !defined(__ANDROID__)
  if(getifaddrs(&probe->padaptorinfo)) {
    doutf(DHIGH, "wsa_probe_begin: getifadds failed %d", errno);
    return 0;
  }
#else
  probe->padaptorinfo = 1;
#endif

  for(probe->padaptor = probe->padaptorinfo; probe->padaptor && (probe->socketcount < arraysize(probe->socket)); ) {
    ipaddr = 0;
    ipmask = 0;

#ifdef _WIN32
    doutf(DLOW, "IP Type:    %d\n", probe->padaptor->Type);
    doutf(DLOW, "IP Address: %s\n", probe->padaptor->IpAddressList.IpAddress.String);
    doutf(DLOW, "IP Mask:    %s\n", probe->padaptor->IpAddressList.IpMask.String);

    switch(probe->padaptor->Type) {
    case MIB_IF_TYPE_ETHERNET:
    case IF_TYPE_IEEE80211:
      ipaddr = inet_addr(probe->padaptor->IpAddressList.IpAddress.String);
      ipmask = inet_addr(probe->padaptor->IpAddressList.IpMask.String);
      break;
    }
#elif defined(__ANDROID__)
    /* getifaddr is not included in android NDK, try to get the first wlan/eth0 interface of the device */
    FILE * fp = fopen("/proc/net/route", "r");
    char buffer[1024];
    int  gateway,pos;
    if(fp) {
      fread(buffer, 1, sizeof(buffer), fp);
      fclose(fp);
      while(pos < sizeof(buffer) && !ipaddr && !ipmask) {
        if(!strncmp(&buffer[pos], "wlan0", 5) || !strncmp(&buffer[pos], "eth0", 4)) {
          //{ char t[32]; memset(t, 0, sizeof(t)); strncpy(t, &buffer[pos], 16); printf("buffer '%s'\n", t); }
          while(pos < sizeof(buffer) && ((buffer[pos] != '\t') && (buffer[pos] != '\n'))) pos++; if(buffer[pos] == '\t') pos++; 
          ipaddr = strtoul(&buffer[pos], 0, 16);
          while(pos < sizeof(buffer) && ((buffer[pos] != '\t') && (buffer[pos] != '\n'))) pos++; if(buffer[pos] == '\t') pos++; 
          gateway = strtol(&buffer[pos], 0, 16);
          while(pos < sizeof(buffer) && ((buffer[pos] != '\t') && (buffer[pos] != '\n'))) pos++; if(buffer[pos] == '\t') pos++; 
          while(pos < sizeof(buffer) && ((buffer[pos] != '\t') && (buffer[pos] != '\n'))) pos++; if(buffer[pos] == '\t') pos++; 
          while(pos < sizeof(buffer) && ((buffer[pos] != '\t') && (buffer[pos] != '\n'))) pos++; if(buffer[pos] == '\t') pos++; 
          while(pos < sizeof(buffer) && ((buffer[pos] != '\t') && (buffer[pos] != '\n'))) pos++; if(buffer[pos] == '\t') pos++; 
          while(pos < sizeof(buffer) && ((buffer[pos] != '\t') && (buffer[pos] != '\n'))) pos++; if(buffer[pos] == '\t') pos++; 
          ipmask = strtol(&buffer[pos], 0, 16);
          if(gateway) {
            ipaddr = 0;
            ipmask = 0;
          }
        } else {
          while(pos < sizeof(buffer) && (buffer[pos] != '\n')) pos++; 
        }
        pos++;
      }
    }
#else
    if(probe->padaptor->ifa_addr->sa_family == AF_INET) {
       char tmp[NI_MAXHOST];
       int result = getnameinfo(probe->padaptor->ifa_addr, sizeof(struct sockaddr_in), tmp, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
       if(result == 0) {
         ipaddr =  inet_addr(tmp);
       }
       result = getnameinfo(probe->padaptor->ifa_netmask, sizeof(struct sockaddr_in), tmp, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
       if(result == 0) {
         ipmask = inet_addr(tmp);
       }
       if(!strcmp(probe->padaptor->ifa_name, "lo")) {
         ipmask = ipaddr = 0;
       }
    }
    doutf(DLOW, "Interface:  %s\n", probe->padaptor->ifa_name);
    doutf(DLOW, "IP Type:    %d\n", probe->padaptor->ifa_addr->sa_family);
#endif
    doutf(DLOW, "IP Address: %d.%d.%d.%d\n", (ipaddr)&0xff, (ipaddr>>8)&0xff, (ipaddr>>16)&0xff, (ipaddr>>24)&0xff);
    doutf(DLOW, "IP Mask:    %d.%d.%d.%d\n", (ipmask)&0xff, (ipmask>>8)&0xff, (ipmask>>16)&0xff, (ipmask>>24)&0xff);

    if(ipaddr && ipmask) {
      struct sockaddr_in sendaddr;
      int result;
      SOCKET sendsocket;
      unsigned char sendbuffer[256];
      int sendbuffersize = 0;
      int broadcast = 1;

      sendsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if(sendsocket < 0) {
        doutf(DMED, "wsa_probe_begin: socket(sendsocket) failed %d\n", WSAGetLastError());
        continue;
      }

      if(setsockopt(sendsocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) != 0) {
         doutf(DMED, "wsa_probe_begin: setsockopt((sendsocket, SOL_SOCKET, SO_BROADCAST, %d) failed %d\n", broadcast, WSAGetLastError());
         closesocket(sendsocket);
         continue;
       }

      memset((void *)&sendaddr, 0, sizeof(sendaddr));
      sendaddr.sin_family      = AF_INET;
      sendaddr.sin_addr.s_addr = ipaddr | ~ipmask;
      sendaddr.sin_port        = htons(18331);

      sendbuffersize = 0;
      sendbuffer[sendbuffersize++] = 0x93;
      sendbuffer[sendbuffersize++] = 0x31;
      sendbuffer[sendbuffersize++] = 0x55;
      sendbuffer[sendbuffersize++] = 0x55;
      sendbuffer[sendbuffersize++] = 0;
      sendbuffer[sendbuffersize++] = 0;
      sendbuffer[sendbuffersize++] = 0;
      sendbuffer[sendbuffersize++] = 2;

      result = sendto(sendsocket, (char*)&sendbuffer[0], sendbuffersize, 0, (SOCKADDR *)&sendaddr, sizeof(sendaddr));
      if(result < 0) {
        doutf(DMED, "wsa_probe_begin: sendto(sendsocket) failed %d\n", WSAGetLastError());
        closesocket(sendsocket);
        continue;
      }

      probe->socket[probe->socketcount++] = sendsocket;
    }

#ifdef _WIN32
    probe->padaptor = probe->padaptor->Next;
#elif defined(__ANDROID__)
    probe->padaptor = 0;
#else
    probe->padaptor = probe->padaptor->ifa_next;
#endif
  }

  return probe;
}

void wsa_probe_end(void * handle)
{
  wsa_probe_handle * probe = (wsa_probe_handle*)handle;
  int i, result;

  for(i = 0; i < probe->socketcount; i++) {
    result = closesocket(probe->socket[i]);
    if(result < 0) {
      doutf(DMED, "wsa_probe_end: closesocket(sendsocket) failed %d\n", WSAGetLastError());
    }
  }

#ifdef _WIN32
  free(probe->padaptorinfo);
#elif !defined(__ANDROID__)
  freeifaddrs(probe->padaptorinfo);
#endif

  free(handle);
}


int wsa_probe_poll(void * handle, int timeout, char * ipaddr, char * device, char * serial, char * version, int stringsize)
{
  wsa_probe_handle * probe = (wsa_probe_handle*)handle;
  unsigned char readbuffer[256];
  socklen_t readaddrsize;
  struct timeval tv;
  SOCKADDR readaddr;
  SOCKET maxsocket;
  int errorcode;
  int i;
  int result;
  fd_set fdsetr;
  int found = 0;

  memset(readbuffer, 0, sizeof(readbuffer));
  memset(&readaddr, 0, sizeof(readaddr));

  readaddrsize = sizeof(readaddr);

  tv.tv_sec  =  timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;

  maxsocket = 0;
  FD_ZERO(&fdsetr);
  for(i = 0; i < probe->socketcount; i++) {
    FD_SET(probe->socket[i], &fdsetr);
    if(probe->socket[i] > maxsocket) {
      maxsocket = probe->socket[i];
    }
  }

  result = select((int)maxsocket + 1, &fdsetr, 0, 0, &tv);
  if(result > 0) {
    for(i = 0; i < probe->socketcount; i++) {
      if(FD_ISSET(probe->socket[i], &fdsetr)) {
        doutf(DLOW, "wsa_probe_poll: select : %d \n", i);

        errorcode = 0;
        result = recvfrom(probe->socket[i], (char*)&readbuffer[0], sizeof(readbuffer), 0, &readaddr, &readaddrsize);
        doutf(DLOW, "wsa_probe_poll: recvfrom : %d \n", result);
        if(result < 0) {
#ifdef _WIN32
          errorcode = WSAGetLastError();
#else
          errorcode = errno;
          if(errorcode == EAGAIN) {
            errorcode = ETIMEDOUT;
          }
#endif
          if(errorcode != ETIMEDOUT) {
            doutf(DHIGH, "wsa_probe_poll: recvfrom(sendsocket) failed %d %d\n", errno, errorcode);
          }
        } else {
          if(readaddr.sa_family ==  SOCK_DGRAM) {
            switch(ntohl(*(unsigned int*)readbuffer)) {
            case 0x93316666:
              sprintf(ipaddr, "%d.%d.%d.%d", (unsigned char)readaddr.sa_data[2], (unsigned char)readaddr.sa_data[3], (unsigned char)readaddr.sa_data[4], (unsigned char)readaddr.sa_data[5]);
              strncpy(device,  (char*)&readbuffer[0x08], 0x10); device[0x10] = 0;
              strncpy(serial,  (char*)&readbuffer[0x18], 0x10); serial[0x10] = 0;
              strncpy(version, (char*)&readbuffer[0x28], 0x10); version[0x10] = 0;
              found = 1;
              break;
            case 0x93315555:
              sprintf(ipaddr, "%d.%d.%d.%d", (unsigned char)readaddr.sa_data[2], (unsigned char)readaddr.sa_data[3], (unsigned char)readaddr.sa_data[4], (unsigned char)readaddr.sa_data[5]);
              strcpy(device,  "WSA4000");
              strcpy(serial,  "");
              strcpy(version, "");
              found = 1;
              break;
            }
          }
        }
      }
    }
  }

  return found;
}




#if defined(STANDALONE)
/*

cl -DSTANDALONE wsa_probe.c

or 

gcc -DSTANDALONE wsa_probe.c -o wsa_probe

*/


int main(int argc, char ** argv)
{
  int devcount;
  int found,i;
  char ipaddr[32];
  char device[32];
  char serial[32];
  char version[32];
  int verbose = 0;
  int timeout;

  if(argc > 1) {
    verbose++;
  }
   
#ifdef WIN32
  struct WSAData ws_data;
  WSAStartup(MAKEWORD(2, 2), &ws_data);

  timeBeginPeriod(1);

  LARGE_INTEGER start;
  LARGE_INTEGER now;
  LARGE_INTEGER freq;

  QueryPerformanceCounter(&start);
  QueryPerformanceFrequency(&freq);
  if(!freq.QuadPart) {
    freq.QuadPart = 1;
  }
#endif

  int us;

  void * handle = wsa_probe_begin();

  if(handle) {
    int i;
    for(i = 0; i < 50; i+=2) {
      found = wsa_probe_poll(handle, 1, ipaddr, device, serial, version, 32);

      if(found) {
#ifdef _WIN32  
        QueryPerformanceCounter(&now);
        us = (int)((now.QuadPart - start.QuadPart) * 1000000ull / freq.QuadPart);
#else
        us = i;
#endif

        if(verbose) {
          printf("%06d ", us);
        }
        printf("%-12s %-16s %-12s %-12s\n", device, ipaddr, serial, version);
        devcount++;
      }
    }

    wsa_probe_end(handle);
  }

#ifdef _WIN32
  if(verbose) {
    QueryPerformanceCounter(&now);
    us = (int)((now.QuadPart - start.QuadPart) * 1000000ull / freq.QuadPart);
    printf("%06d ", us);
  }
  WSACleanup();
#endif
}
#endif
