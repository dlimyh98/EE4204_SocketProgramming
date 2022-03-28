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
#include <sys/time.h>

// Port numbers below 1024 are reserved, cannot be used for local transfer on Machine
// Client and Server MUST BE same Port Number, but their IP Addresses can be different!
// Socket = Port + IP Address, which leads to communication between Client and Server
// IP Address used is 127.0.0.1 (loopback IP), so that programs on Machine can talk to themselves WITHOUT having to go out to the Internet and then coming back through the same router
#define MYUDP_PORT 5350
#define DATALEN 500
#define BATCH_SIZE 3
#define BUFSIZE 60000

struct ackPacket
{
    uint8_t sequenceNumber;
    uint8_t packetLength;
};
