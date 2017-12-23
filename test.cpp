#include "tinySockets.h"

#include <stdio.h>

int main() {
    

    int sock = startConnecting( "172.217.5.196", 80 );
    

    if( sock != -1 ) {
        printf( "Started connecting on socket %d\n", sock );    

        int connected = 0;
        while( connected == 0 ) {
            
            connected = isConnected( sock );
            }
        
        if( connected == 1 ) {
            printf( "Connected\n" );
            
            closeSocket( sock );
            }
        else {
            printf( "Connection error\n" );
            }
        }
    
    
    return 1;
    }
