#include "tinySockets.h"


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>




int startConnecting( const char *inIPAddress, int inPort ) {
    
    static struct in_addr internetAddress;
    
    int error = inet_aton( inIPAddress, &internetAddress );
    
    if( error == 0 ) {
        printf( "startConnecting:  Failed to process IP address %s\n", 
                inIPAddress );
        return -1;
        }

    int socketID = socket( AF_INET, SOCK_STREAM, 0 );	

    union sock {
            struct  sockaddr s;

            struct  sockaddr_in i;
		} sock;
    
    
    sock.i.sin_family = AF_INET;
	sock.i.sin_port = htons( inPort );
	sock.i.sin_addr = internetAddress;


    int ret = fcntl( socketID, F_SETFL, O_NONBLOCK );
	
	if( ret < 0 ) {
        printf( 
            "startConnecting:  failed to put socket in non-blocking mode\n" );
        close( socketID );
        return -1;
        }


    ret = connect( socketID, &( sock.s ), sizeof( struct sockaddr ) );
	
	if( ret < 0 && errno != EINPROGRESS) {
        printf( 
            "startConnecting:  connect failed (error= %s)\n",
            strerror( errno ) );
        close( socketID );
        return -1;
        }
    
    return socketID;
    }



int isConnected( int inSocket ) {
    fd_set fsr;
	struct timeval tv;
	int val;
    socklen_t len;

	FD_ZERO( &fsr );
	FD_SET( inSocket, &fsr );

    // check if connection event waiting right now
    // timeout of 0
    tv.tv_sec = 0;
	tv.tv_usec = 0;    

	int ret = select( inSocket + 1, NULL, &fsr, NULL, &tv );

	if( ret == 0 ) {
		// timeout
		return 0;
        }
    

    // no timeout
    // error?

	len = 4;
	ret = getsockopt( inSocket, SOL_SOCKET, SO_ERROR, &val, &len );
	
	if( ret < 0 ) {
		// error
        return -1;
        }

	if( val != 0 ) {
        // error
		return -1;
        }
	
    // success
    


    // enable no-delay on newly connected socket
    int flag = 1;
    setsockopt( inSocket,
                IPPROTO_TCP,
                TCP_NODELAY,
                (char *) &flag,
                sizeof(int) );
    
    return 1;
    }







void closeSocket( int inSocket ) {
    close( inSocket );
    }
