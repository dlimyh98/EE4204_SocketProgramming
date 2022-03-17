#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

// Port numbers below 1024 are reserved, cannot be used for local transfer on Machine
// Client and Server MUST BE same Port Number, but their IP Addresses can be different!
// Socket = Port + IP Address, which leads to communication between Client and Server
// IP Address used is 127.0.0.1 (loopback IP), so that programs on Machine can talk to themselves WITHOUT having to go out to the Internet and then coming back through the same router
#define MYTCP_PORT 4950
#define MYUDP_PORT 5350
#define MAXSIZE 50
