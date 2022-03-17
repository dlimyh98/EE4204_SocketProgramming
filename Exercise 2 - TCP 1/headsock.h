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

#define NEWFILE (O_WRONLY|O_CREAT|O_TRUNC)
#define MYTCP_PORT 4950
#define MYUDP_PORT 5350
#define MAXSIZE 30008
#define MAXLEN 30000
#define MAXINT 0x7fffffff
#define BUFSIZE 31000
#define N 1
#define HEADLEN 8


struct dataPacket
{
    uint32_t sequenceNumber;
    uint32_t packetLength;
    char packetData[MAXLEN];
};

struct ackPacket
{
    uint8_t sequenceNumber;
    uint8_t packetLength;
};
