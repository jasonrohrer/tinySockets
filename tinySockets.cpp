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
#include <sys/epoll.h>


int epollHandle = -1;
int numPolledSockets = 0;


static void addSockToPoll( int inSocket, char inServer = false ) {
    
    if( epollHandle == -1 ) {
        // starting size ignored by newer kernels
        epollHandle = epoll_create( 10 );
        }

    
    struct epoll_event ev;
    
    if( inServer ) {
        ev.events = EPOLLIN;
        }
    else {
        ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
        }
    

    // clear entire union to suppress valgrind uninit errors on platforms
    // with 32-bit pointers
    ev.data.u64 = 0;
    
    ev.data.fd = inSocket;
 
    
    int result = epoll_ctl( epollHandle, EPOLL_CTL_ADD, inSocket, &ev );
    
    if( result == 0 ) {
        numPolledSockets++;
        }
    }

    




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

    while( ret < 0 && errno == EINTR ) {
        // interrupted during select
        // try again
        ret = select( inSocket + 1, &fsr, NULL, NULL, &tv );
        }


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
    
    
    addSockToPoll( inSocket );
    
    return 1;
    }



int waitForSocketEvent( int inTimeoutInMilliseconds ) {
    
    if( epollHandle == -1 ) {
        return -1;
        }
    
    struct epoll_event returnedEvents[1];

    int numEvents = epoll_wait( epollHandle, returnedEvents, 1, 
                                inTimeoutInMilliseconds );

    if( numEvents == 0 ) {
        return 0;
        }
    else if( numEvents == 1 ) {
        return returnedEvents[0].data.fd;
        }
    else {
        return -1;
        }
    }



int socketSend( int inSocket, unsigned char *inBytes, int inNumToSend ) {
    return send( inSocket, inBytes, inNumToSend, 0 );
    }




int socketReceive( int inSocket, unsigned char *inBytes, 
                   int inMaxNumToReceive ) {
    
    // select first to make sure data is there
    // and to distinguish data from error

    fd_set fsr;
    struct timeval tv;
    
    
    FD_ZERO( &fsr );
    FD_SET( inSocket, &fsr );
 
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    int ret = select( inSocket + 1, &fsr, NULL, NULL, &tv );

    
    while( ret < 0 && errno == EINTR ) {
        // interrupted during select
        // try again
        ret = select( inSocket + 1, &fsr, NULL, NULL, &tv );
        }


    if( ret == 1 ) {
        
        int numReceived = recv( inSocket, inBytes, inMaxNumToReceive, 0 );

        if( numReceived == 0 ) {
            // select said sock was ready, but no bytes there
            // must be an error
            return -1;
            }

        
        if( ret == -1 && 
            ( errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) ) {
            // select came back 1, but then our recv operation was interrupted
            // or would block
        
            // treat like a timeout
            return 0;
            }

        return numReceived;
        }
    else if( ret == 0 ) {
        // timeout
        return 0;
        }
    else {
        // error selecting socket
        return -1;
        }
    }






void closeSocket( int inSocket ) {
    
    if( epollHandle != -1 ) {
        struct epoll_event ev;
        
        int result = epoll_ctl( epollHandle, EPOLL_CTL_DEL, inSocket, &ev );

        if( result == 0 ) {
            numPolledSockets--;
            }
        
        if( numPolledSockets == 0 ) {
            close( epollHandle );
            epollHandle = -1;
            }
        }
    
    close( inSocket );
    }



int startServer( int inPort, int inBacklogSize ) {

    int error;
    
    int sockID = socket( AF_INET, SOCK_STREAM, 0 );
    
    if( sockID == -1 ) {
        printf( "startServer:  Failed to construct a socket\n" );
        return -1;
        }
    

    // this setsockopt code partially copied from gnut
    
    // set socket option to enable reusing addresses so that the socket is
    // unbound immediately when the server is shut down
    // (otherwise, rebinding to the same port will fail for a while
    //  after the server is shut down)
    int reuseAddressValue = 1;
    error = setsockopt( sockID,
                        SOL_SOCKET,      // socket-level option
                        SO_REUSEADDR,    // reuse address option
                        &reuseAddressValue,  // value to set for this option
                        sizeof( reuseAddressValue) );  // size of the value

    
    if( error == -1  ) {
        printf( "startServer:  Failed to set socket options\n" );
        return -1;
        }
    

    union sock {
            struct  sockaddr s;
            
            struct  sockaddr_in i;
        } sock;

    sock.i.sin_family = AF_INET;
    sock.i.sin_port = htons( inPort );
    sock.i.sin_addr.s_addr = INADDR_ANY;
    
    error = bind( sockID,  &( sock.s ), sizeof( sockaddr ) );
    
    if( error == -1  ) {
        printf( "startServer:  Bad socket bind, port %d\n", inPort );
        return -1;
        }
    
    
    // start listening for connections
    error = listen( sockID, inBacklogSize );
    if( error == -1 ) {
        printf( "startServer:: Bad socket listen\n" );
        return -1;
        }


    addSockToPoll( sockID, true );
    
    return sockID;
    }




int acceptConnection( int inServerSocket ) {
    
    fd_set rfds;
    struct timeval tv;
    
    // insert our socket descriptor into this set
    FD_ZERO( &rfds );
    FD_SET( inServerSocket, &rfds );

    tv.tv_sec = 0;
    tv.tv_usec = 0;
        
    int retval = select( inServerSocket + 1, &rfds, NULL, NULL, &tv );
    
    while( retval < 0 && errno == EINTR ) {
        // interrupted during select
        // try again
        retval = select( inServerSocket + 1, &rfds, NULL, NULL, &tv );
        }

    if( retval == 0 ) {
        // timeout
        return 0;
        }
    else if( retval == -1 ) {
        printf( "acceptConnection:  select on server socket failed\n" );
        return -1;
        }
    

    int acceptedID = accept( inServerSocket, NULL, NULL );

    if( acceptedID == -1 ) {
        printf( "acceptConnection:: Failed to accept a network connection.\n" );
        return -1;
        }


    // enable no-delay on newly connected socket
    int flag = 1;
    setsockopt( acceptedID,
                IPPROTO_TCP,
                TCP_NODELAY,
                (char *) &flag,
                sizeof(int) );

    addSockToPoll( acceptedID );
    
    return acceptedID;
    }
