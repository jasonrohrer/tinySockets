// placed in the public domain by Jason Rohrer


// change this value based on your application
// internal memory is statically allocated with no mallocs
#define MAX_LIVE_SOCKETS  1024




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
// *outServerSocket is set to:
//   true if the triggered socket is a server socket
//   or
//   false if the triggered socket is a connected socket
// 
// returns:
//   socket handle for socket with data that can be received 
//   or
//   server socket handle for server socket that has a new connection waiting
//   or
//  -1 on error 
int waitForSocketEvent( int inTimeoutInMilliseconds, char *outServerSocket );



// returns:
//   number of bytes sent (including, possibly, 0 if socket is blocked)
//  -1 on error
int send( int inSocket, unsigned char *inBytes, int inNumToSend );



// returns:
//   number of bytes received
//  -1 on error
int receive( int inSocket, unsigned char *inBytes, int inMaxNumToReceive );



// closes either a client or a server socket
void closeSocket( int inSocket );






////////////////////// 
// server functions //
//////////////////////



// returns:
//   server socket handle
//   or
//  -1 on error
int startServer( int inPort );



// returns:
//   socket handle for new connection
//   or
//   0 on timeout
//  -1 on error
int acceptConnection( int inServerSocket );
