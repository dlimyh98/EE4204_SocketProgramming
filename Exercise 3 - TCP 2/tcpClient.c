/*******************************
 * TCP Client
 ********************************/
#include "headsock.h"

float clientFunc(FILE *fp, int sockfd, long *packetSize);
void calcTransmissionTimeInterval(struct timeval* out, struct timeval* in);

int main(int argc, char **argv)
{
    int sockfd, ret;
    struct sockaddr_in ser_addr;
    struct hostent* userHostEnt;
    struct in_addr** userAddress;
    FILE *fp;

    /**************************** Parsing Input **********************************/
    if (argc != 2) {
        printf("Parameters not match");
    }

    userHostEnt = gethostbyname(argv[1]);
    if (userHostEnt == NULL) {
        printf("Error when gethostby name");
        exit(0);
    }

    /**************************** Server Information (where Client is going to send to) **********************************/
    printf("Canonical name: %s\n", userHostEnt->h_name);
    for (char** pptr = userHostEnt->h_aliases; *pptr != NULL; pptr++)
        printf("The aliases name is: %s\n", *pptr);

    switch(userHostEnt->h_addrtype)
    {
        case AF_INET:
            printf("AF_INET\n");
            break;
        default:
            printf("Unknown addrtype\n");
            break;
    }

    /**************************** Creating Socket **********************************/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);                           //create the socket
    if (sockfd <0)
    {
        printf("error in socket");
        exit(1);
    }

    /**************************** Update Address Parameters of Server Address **********************************/
    char** addressList = userHostEnt->h_addr_list;    // NULL terminated array of in_addr structs
    userAddress = (struct in_addr**)addressList;      // cast to (struct in_addr**)

    ser_addr.sin_family = AF_INET;                                                      
    ser_addr.sin_port = htons(MYTCP_PORT);
    memcpy(&(ser_addr.sin_addr.s_addr), *userAddress, sizeof(struct in_addr));
    bzero(&(ser_addr.sin_zero), 8);

    /**************************** Client Address makes connection request to Socket (NOT IN UDP) **********************************/
    ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));
    if (ret != 0) {
        printf ("Connection failed\n"); 
        close(sockfd); 
        exit(1);
    }

    /**************************** Open local file to read data **********************************/
    if((fp = fopen ("myfile.txt","r+t")) == NULL)
    {
        printf("File doesn't exist\n");
        exit(0);
    }

    /**************************** Perform Transmission **********************************/
    float transmissionTime, transmissionRate;
    long fileToTransmitSize;
    transmissionTime = clientFunc(fp, sockfd, &fileToTransmitSize); // perform transmission of file and receiving of ACK

    // Calculate parameters of data transfer
    if (transmissionTime != -1) {
        transmissionRate = fileToTransmitSize / (float)transmissionTime;
        printf("Avg Time(ms): %.3f\nAvg Data sent: %ld bytes\nAvg Data rate: %f Kbytes/s\n", transmissionTime, (long)fileToTransmitSize, transmissionRate);
    }

    close(sockfd);
    fclose(fp);
    exit(0);
}

float clientFunc(FILE *fp, int sockfd, long *fileToTransmitSize)
{
    struct ackPacket acksPacket;             // user defined
    struct timeval sendt, recvt;             // sys/time.h

    char *buffer;                // buffer for entire file
    char sendPacket[DATALEN];    // packet used for sending
    long fileSize;               // size of file to be sent (NOT PACKET)
    int sendPacketSize;          // size of packet used for transmission (to send file)
    long numBytesSent;           // number of bytes Client sent to Server (in one send() call)
    long numBytesReceived;       // number of bytes Client received from Server (in one recv() call)
    long sendBytesCounter = 0;   // counter to keep track of number of bytes Client has sent SO FAR
    float transmissionTime = 0.0;

    // Determine the length of file to send
    fseek (fp , 0 , SEEK_END);
    fileSize = ftell (fp);
    rewind (fp);
    printf("The file size is %d bytes\n", (int)fileSize);
    printf("The packet size is %d bytes\n",DATALEN);

    // Allocate memory to contain the whole file.
    buffer = (char *) malloc (fileSize);
    if (buffer == NULL) exit (2);

    // Copy the file into the buffer.
    fread (buffer, 1, fileSize, fp);
    buffer[fileSize] ='\0';      // append end-byte of file to be null-terminating character (used to signal when to stop)
    // assert that entire file is loaded into buffer

    gettimeofday(&sendt, NULL);

    // While whole file is not transmitted
    while(sendBytesCounter <= fileSize)
    {
        // verify if the size of LAST PACKET is less than or equal to DATALEN. If so, use the size of that packet as DATALEN instead.
        if ((fileSize + 1 - sendBytesCounter) <= DATALEN)
            sendPacketSize = fileSize + 1 - sendBytesCounter;
        else 
            sendPacketSize = DATALEN;

        // send a packet
        memcpy(sendPacket, (buffer + sendBytesCounter), sendPacketSize);
        numBytesSent = send(sockfd, &sendPacket, sendPacketSize, 0);

        if(numBytesSent == -1) {
            printf("Error sending from Client to Server!");
            exit(1);
        }

        sendBytesCounter += sendPacketSize;
    }

    *fileToTransmitSize = sendBytesCounter;

    // wait to receive ACK from Server
    if ((numBytesReceived = recv(sockfd, &acksPacket, 2, 0)) == -1)
    {
        printf("Error when receiving ACK from Server\n");
        exit(1);
    }

    if (acksPacket.sequenceNumber != 1|| acksPacket.packetLength != 0)
        printf("ACK from Server is corrupted, possible error in transmission\n");

    gettimeofday(&recvt, NULL);
    calcTransmissionTimeInterval(&recvt, &sendt);
    transmissionTime += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
    return(transmissionTime);
}


void calcTransmissionTimeInterval(struct timeval* out, struct timeval* in)
{
    if ((out->tv_usec -= in->tv_usec) <0)
    {
        --out ->tv_sec;
        out ->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}
