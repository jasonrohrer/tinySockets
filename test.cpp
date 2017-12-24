#include "tinySockets.h"

#include <stdio.h>

int main() {
    
    int serverSock = -1;
    int inboundSock = -1;
    int sock = -1;

    serverSock = startServer( 8000, 10 );

    
    printf( "Waiting for inbound connection on port 8000\n" );
            
    int waitResult = 0;
    while( waitResult == 0 ) {
        waitResult = waitForSocketEvent( 500 );
        }
    
    if( waitResult == serverSock ) {
        printf( "Got server event\n" );
        
        inboundSock = acceptConnection( serverSock );

        if( inboundSock != -1 ) {
            printf( "Got connection\n" );




            sock = startConnecting( "127.0.0.1", 8001 );
    
            
            if( sock != -1 ) {
                printf( "Started connecting on socket %d\n", sock );    
                
                int connected = 0;
                while( connected == 0 ) {
                    
                    connected = isConnected( sock );
                    }
        
                if( connected == 1 ) {
                    printf( "Connected\n" );

                    unsigned char buffer[513];
                    
                    int numReceived = 0;
                    
                    while( numReceived >= 0 ) {
                        
                        waitResult = 0;
                        
                        while( waitResult == 0 ) {
                            waitResult = waitForSocketEvent( 500 );    
                            }
                        
                        
                        if( waitResult == inboundSock ) {
                            numReceived = socketReceive( inboundSock, 
                                                         buffer, 512 );
                            
                            if( numReceived > 0 ) {
                                int numSent = 
                                    socketSend( sock, buffer, numReceived );
                                
                                if( numSent != numReceived ) {
                                    printf( "Send failed\n" );
                                    numReceived = -1;
                                    }
                                }
                            }
                        else if( waitResult == sock ) {
                            numReceived = socketReceive( sock, 
                                                         buffer, 512 );
                            
                            if( numReceived > 0 ) {
                                buffer[ numReceived ] = '\0';
                                printf( "Received from outbound sock: %s\n",
                                        buffer );
                                }
                            }
                        }
                    printf( "Connection broken\n" );
                    }
                else {
                    printf( "Failed to connect\n" );
                    }
                }
            }
        }

    if( sock != -1 ) {
        closeSocket( sock );
        }
    if( inboundSock != -1 ) {
        closeSocket( inboundSock );
        }
    if( serverSock != -1 ) {
        closeSocket( serverSock );
        }

    return 1;
    }
