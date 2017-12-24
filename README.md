# tinySockets
Simple single-file C wrapper code for thread-free, non-blocking sockets on Linux.

Wraps functionality in <sys/socket.h>

No mallocs internally (except for any that underlying socket implementation does).

Uses epoll for efficient handling of many non-blocking sockets.


The Nagle algorithm is disabled (TCP_NODELAY set).  I expect user code to break outbound messages into chunks in an intelligent way (don't depend on the send call to handling buffering). 
