# tinySockets
Simple single-file C wrapper code for thread-free, non-blocking sockets on Linux.

Wraps functionality in <sys/socket.h>

No mallocs internally (except for any that underlying socket implementation does).  Size of static memory footprint confirgurable in header file based on max number of connections.

Uses epoll for efficient handling of many non-blocking sockets.
