/*******************************
 * TCP Client
 ********************************/
#include "headsock.h"
float clientFunc(FILE *fp, int sockfd, long *packetSize);                      // Packet transmission function
void calcTransmissionTimeInterval(struct timeval* out, struct timeval* in);    // Calculate the transmission interval between out and in


int main(int argc, char **argv)
{
    int sockfd, ret;
    struct sockaddr_in ser_addr;
    struct hostent *userHostEnt;
    struct in_addr **userAddress;
    FILE *fp;

    /******************************* Parsing Input ********************************************/
    if (argc != 2) {
        printf("Parameters not match");
    }

    userHostEnt = gethostbyname(argv[1]);
    if (userHostEnt == NULL) {
        printf("Error when gethostby name");
        exit(0);
    }

    /******************************* Server Information (where Client is going to send to) ********************************************/
    printf("Canonical name: %s\n", userHostEnt->h_name);
    for (char** pptr = userHostEnt->h_aliases; *pptr != NULL; pptr++)
        printf("The aliases name is: %s\n", *pptr);

    switch(userHostEnt->h_addrtype)
    {
        case AF_INET:
            printf("AF_INET\n");
            break;
        default:
            printf("unknown address type\n");
            break;
    }

    /******************************* Creating Socket ********************************************/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <0)
    {
        printf("Error in socket");
        exit(1);
    }

    /*********************************** Update Address Parameters of Server Address *************************************************/
    char** addressList = userHostEnt->h_addr_list;       // NULL-terminated array of in_addr structs
    userAddress = (struct in_addr**)addressList;         // cast to (struct in_addr**)

    ser_addr.sin_family = AF_INET;                                                      
    ser_addr.sin_port = htons(MYTCP_PORT);
    memcpy(&(ser_addr.sin_addr.s_addr), *userAddress, sizeof(struct in_addr));
    bzero(&(ser_addr.sin_zero), 8);

    /*********************************** Client Address makes connection request to Socket (NOT IN UDP) *************************************************/
    ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr)); 
    if (ret != 0) {
        printf ("Connection failed\n"); 
        close(sockfd); 
        exit(1);
    }

    /*********************************** Open local file to read data (Different from normal TCP) *************************************************/
    if((fp = fopen ("myfile.txt","r+t")) == NULL)
    {
        printf("File doesn't exist\n");
        exit(0);
    }

    /*********************************** Perform Transmission *************************************************/
    float transmissionTime, transmissionRate;
    long packetSize;
    transmissionTime = clientFunc(fp, sockfd, &packetSize); // perform transmission of file and receiving of ACK

    // Calculate parameters of data transfer
    if (transmissionTime != -1)	{
        transmissionRate = packetSize / (float)transmissionTime; 
        printf("Avg Time(ms): %.3f\nAvg Data sent: %d bytes\nAvg Data rate: %f Kbytes/s\n", transmissionTime, (int)packetSize, transmissionRate);
    }

    close(sockfd);
    fclose(fp);
    exit(0);
}

float clientFunc(FILE *fp, int sockfd, long *packetSize)
{
    struct dataPacket transmissionPacket;    // user defined
    struct ackPacket acksPacket;             // user defined
    struct timeval sendt, recvt;             // sys/time.h

    long fileSize;
    int numBytesSent;
    int numBytesReceived;
    float transmissionTime = 0.0;

    // Determine the length of the file to send
    fseek (fp, 0, SEEK_END);
    fileSize = ftell(fp);
    *packetSize = fileSize;
    rewind (fp);
    printf("The file size is %d bytes\n", (int)fileSize);

    // load file to be sent into transmissionPacket packetData field
    fread (transmissionPacket.packetData, 1, fileSize, fp);

    // adjust transmissionPacket parameters
    transmissionPacket.packetLength = fileSize;  // set transmissionPacket packetLength to fileSize
    transmissionPacket.sequenceNumber = 0;       // if sending multiple files, you can change this

    // get current time (sys/time.h)
    gettimeofday(&sendt, NULL);

    // use TCP send, and then must wait for acknowledgement
    // sending transmissionPacket, but add HEADLEN to transmissionPacket size
    numBytesSent = send(sockfd, &transmissionPacket, (transmissionPacket.packetLength+HEADLEN), 0);		// send the data in ONE packet
    // simplifying assumption that we are able to send() the entire buffer in one go (numBytesSent == fileSize)
    // for any stream-based protocols such as TCP, send may return less than amount requested
    // for datagram protocols such as UDP, send will either fail or send the ENTIRE buffer as a packet (no partial sends are possible)

    if (numBytesSent == -1)	{			
        printf("Client has error sending data to Server\n");
        exit(1);
    }
    else {
        printf("%d bytes of data sent\n", numBytesSent);

    }

    // wait for acknowledgement
    if ((numBytesReceived = recv(sockfd, &acksPacket, 2, 0)) == -1) {	        // receive ACK or NACK
        printf("Client has error receiving data from Server\n");
        exit(1);
    }

    // if Client receives ACK (packetLength and sequenceNumber were set on Server side)
    if ((acksPacket.packetLength == 0) && (acksPacket.sequenceNumber == 1))
    {
        gettimeofday(&recvt, NULL);
        calcTransmissionTimeInterval(&recvt, &sendt);
        transmissionTime += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
        return(transmissionTime);
    }
    else {
        printf("Error in transmission\n");
        return(-1);
    }
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
