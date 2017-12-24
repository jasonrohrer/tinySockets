// placed in the public domain by Jason Rohrer




//////////////////////
// client functions //
//////////////////////



// returns:
//   socket handle, or
//   -1 on error
int startConnecting( const char *inIPAddress, int inPort );



// check whether a socket created through startConnecting has connected
// Note that after 1 is returned from this call, future calls will return
// undefined results (this function does not determine if the socket is 
// STILL connected later).
//
// returns:
//   1 if connected
//   0 if still trying to connect
//  -1 on error
int isConnected( int inSocket );






/////////////////////////////////
// client and server functions //
/////////////////////////////////


// Waits for a socket to have receivable data or a server socket
// to have a new connection
// 
// returns:
//   socket handle for socket with data that can be received 
//   or
//   server socket handle for server socket that has a new connection waiting
//   or
//   0 on timeout
//   or
//  -1 on error 
int waitForSocketEvent( int inTimeoutInMilliseconds );



// returns:
//   number of bytes sent (including, possibly, 0 if socket is blocked)
//  -1 on error
int socketSend( int inSocket, unsigned char *inBytes, int inNumToSend );



// returns:
//   number of bytes received
//  -1 on error
int socketReceive( int inSocket, unsigned char *inBytes, 
                   int inMaxNumToReceive );



// closes either a client or a server socket
void closeSocket( int inSocket );






////////////////////// 
// server functions //
//////////////////////



// returns:
//   server socket handle
//   or
//  -1 on error
int startServer( int inPort, int inBacklogSize );



// returns:
//   socket handle for new connection
//   or
//   0 on timeout
//  -1 on error
int acceptConnection( int inServerSocket );
