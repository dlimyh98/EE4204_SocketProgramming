/*******************************
 * UDP Client (data is transmitted from here)
 ********************************/
#include "headsock.h"

void calcTransmissionTimeInterval(struct timeval* out, struct timeval* in);
long sendStringOnClient(FILE* fp, int sockfd, struct sockaddr* serverAddress, int serverAddressLength, long* fileToTransmitSize);


int main(int argc, char *argv[])
{
    /************************************************** Variable Declarations *****************************************************************/
    struct sockaddr_in server_address;    // Client will send to this address
    int len, sockfd;
    struct hostent* userHostEnt;          // struct hostent is used by functions to store information about a given host (can be Client or Server)
    struct in_addr** userServerAddress;   // struct in_addr represents IPv4 Internet Address
    FILE* fp;

    /************************************************** Parsing Input *****************************************************************/
    if (argc!= 2)
    {
        printf("parameters not match.");
        exit(0);
    }

    if ((userHostEnt=gethostbyname(argv[1])) == NULL) {
        printf("error when gethostbyname");
        exit(0);
    }

    /**************************************** Printing Host Information (optional) *********************************************/
    printf("Official name of host: %s\n", userHostEnt->h_name);

    for (char** pptr = userHostEnt->h_aliases; *pptr != NULL; pptr++)
        printf("Host aliases name is: %s\n", *pptr);

    switch(userHostEnt->h_addrtype)
    {
        case AF_INET:
            printf("AF_INET\n");
            break;

        default:
            printf("unknown addrtype\n");
            break;
    }

    /**************************************** Create socket for Client (Client will send through this socket) *******************************************************/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("error in socket");
        exit(1);
    }

    /****************************** Update Parameters for Address that Client will send data to ********************************************/
    char** addressList = userHostEnt->h_addr_list;       // NULL-terminated array of in_addr structs
    userServerAddress = (struct in_addr**)addressList;   // cast to (struct in_addr**)

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(MYUDP_PORT);
    memcpy(&(server_address.sin_addr.s_addr), *userServerAddress, sizeof(struct in_addr));
    bzero(&(server_address.sin_zero), 8);

    /**************************** Open local file to read data **********************************/
    fp = fopen("myfile.txt", "r+t");

    if (fp == NULL) {
        printf("File doesn't exist\n");
        exit(0);
    }

    /**************************** Perform Transmission **********************************/
    float transmissionTime, transmissionRate;
    long fileToTransmitSize;
    transmissionTime = sendStringOnClient(fp, sockfd, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in), &fileToTransmitSize);


    // Calculate parameters of data transfer
    if (transmissionTime != -1) {
        transmissionRate = fileToTransmitSize / (float)transmissionTime;
        printf("Avg Time(ms): %.3f\nAvg Data sent: %ld bytes\nAvg Data rate: %f Kbytes/s\n", transmissionTime, fileToTransmitSize, transmissionRate);
    }

    close(sockfd);
    exit(0);
}


long sendStringOnClient(FILE* fp, int sockfd, struct sockaddr* serverAddress, int serverAddressLength, long* fileToTransmitSize)
{
    struct timeval sendt, recvt;
    struct ackPacket acksPacket;

    char* buffer;                             // buffer to ENTIRE file that Client wants to transmit
    char packetToSend[DATALEN];               // base packet that Client will use for transmission
    long fileSize = 0;                        // size of ENTIRE file that Client wants to transmit
    long sendPacketSize;                      // size of packet that Client will use for transmission
    long numBytesSent;                        // number of bytes Client sent to Server (in one send() call)
    long numBytesReceived;                    // number of bytes Client received from Server (in one recv() call)
    long sendBytesCounter = 0;                // counter to keep track of number of bytes Client has sent SO FAR
    float transmissionTime = 0.0;
    int batchNumber = 0;                      // Batches of size n


    // Determine the length of file to send
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    printf("The file size is %d bytes\n", (int)fileSize);
    printf("Client will send %d packets of MAXIMUM %d bytes each in ONE batch\n", BATCH_SIZE, DATALEN);

    // Allocate memory to contain the whole file.
    buffer = (char*)malloc(fileSize);
    if (buffer == NULL) exit(2);

    // Copy the file into the buffer.
    fread(buffer, 1, fileSize, fp);
    buffer[fileSize] = '\0';      // append end-byte of file to be null-terminating character (used to signal when to stop)
    // assert that entire file is loaded into buffer

    gettimeofday(&sendt, NULL);


    // while whole file is not transmitted
    while (sendBytesCounter <= fileSize)
    {
        // send in batches
        for (int i = 0; i < BATCH_SIZE && sendBytesCounter <= fileSize; i++) {

            // verify if the size of LAST PACKET is less than or equal to DATALEN. If so, use the size of that packet as DATALEN instead.
            if ((fileSize + 1 - sendBytesCounter) <= DATALEN) {
                sendPacketSize = fileSize + 1 - sendBytesCounter;
            }
            else {
                sendPacketSize = DATALEN;
            }

            // send to Server
            memcpy(packetToSend, (buffer + sendBytesCounter), sendPacketSize);
            numBytesSent = sendto(sockfd, &packetToSend, sendPacketSize, 0, serverAddress, serverAddressLength);

            if (numBytesSent == -1) {
                printf("Error transmitting from Client to Server\n");
                exit(1);
            }
            else {
                sendBytesCounter += sendPacketSize;
            }

            printf("Client sent %ld bytes to Server, as part of Packet %d of Batch %d\n", numBytesSent, i, batchNumber);
        }

        batchNumber++;

        // wait for ACK (blocking revfrom())
        if ((numBytesReceived = recvfrom(sockfd, &acksPacket, sizeof(acksPacket), 0, serverAddress, &serverAddressLength)) == -1)
        {
            printf("Error when receiving ACK from Server\n");
            exit(1);
        }
        else if (acksPacket.sequenceNumber != 1 || acksPacket.packetLength != 0) {
            printf("ACK from Server is corrupted, error in transmission\n");
        }
    }

    *fileToTransmitSize = sendBytesCounter;

    gettimeofday(&recvt, NULL);
    calcTransmissionTimeInterval(&recvt, &sendt);
    transmissionTime += (recvt.tv_sec) * 1000.0 + (recvt.tv_usec) / 1000.0;
    return(transmissionTime);
}



void calcTransmissionTimeInterval(struct timeval* out, struct timeval* in)
{
    if ((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}
