#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "lib_crc/lib_crc.h"
#include "Packet.hpp"

#define PAYLOAD_MAX 1024                /* Longest payload */

class UDPReceiver {
    protected:
        int sock;                       /* Socket */
        struct sockaddr_in myAddr;      /* Local address */
        struct sockaddr_in senderAddr;  /* Sender address */
    
        unsigned int cliAddrLen;        /* Length of incoming message */
        char payload[PAYLOAD_MAX];      /* Buffer for echo string */
        unsigned short listeningPort;   /* The port to listen to */
        int recvMsgSize;                /* Size of received message */

    public:
        UDPReceiver(void);
        UDPReceiver(unsigned short port);
        void listen( void );
        void init_connection( void );
        void close_connection( void );
};