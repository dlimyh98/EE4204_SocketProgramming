/**************************************
 * UDP Server (data is received here)
 **************************************/
#include "headsock.h"

void receiveStringOnServer(int sockfd, struct sockaddr_in client_addr);

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;    // Server will receive from this Address
    struct sockaddr_in client_addr;    // Server will respond to the Client at this address (after it receives from it)

    // Create Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("error in socket");
        exit(1);
    }

    // Filling in Server Address Parameters
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(MYUDP_PORT);    // Address that Server receives from will have this UDP_PORT number (same as Client's UDP_PORT number)
    server_addr.sin_addr.s_addr = INADDR_ANY;    // Address that Server receives from does not have any specific IP, ALL IPs to local machine accepted (accept 127.0.0.1 in this case)
    bzero(&(server_addr.sin_zero), 8);


    // Associate Socket to some Port, so Server knows which IP-Address/Port pair it should listen to
    // e.g. have Server Socket be bound to Port 80 for Web Server
    if (bind(sockfd, (struct sockaddr* ) &server_addr, sizeof(struct sockaddr)) == -1) { 
        printf("Error in binding");
        exit(1);
    }

    printf("Start receiving\n");

    receiveStringOnServer(sockfd, client_addr);

    close(sockfd);
    exit(0);
}

void receiveStringOnServer(int sockfd, struct sockaddr_in client_addr)
{
    // Unlike TCP, Server does not wait for Client to connect
    // Therefore, Server does not receive Client's address prior to sending or receiving data (yes, Server can send data to Client!)
    // Instead, Server receives information about Client when it receives data from Client (from recv(), stored in client_addr)
    struct ackPacket acksPacket;
    int clientStructLength = sizeof(client_addr);
    FILE* fp;
    int isLastPacket = 0;
    char receiveBuffer[DATALEN];      // incoming message from Client is stored here, when recvfrom() is called
    char fileBuffer[BUFSIZE];         // Server side buffer to store ENTIRE file that Client wants to send (NOT just one packet from Client)
    int numBytesReceived = 0;         // Number of bytes Server has received from Client in one recvfrom() call
    int numBytesSent = 0;             // Number of bytes Server has sent to Client in one sendto() call
    long fileBufferTracker = 0;       // keeps track of where in fileBuffer we should store next packet from Client
    int numPacketsReceived = 0;       // keeps track of how many (out of n) packets received in a batch from Client

    while (!isLastPacket) {
        numBytesReceived = recvfrom(sockfd, (char*)receiveBuffer, DATALEN, 0, (struct sockaddr*)&client_addr, &clientStructLength);

        if (numBytesReceived == -1) {
            printf("Error when Server is receiving from Client\n");
            exit(1);
        }

        printf("Server received %d number of bytes from Client\n", numBytesReceived);

        if (receiveBuffer[numBytesReceived - 1] == '\0') {
            isLastPacket = 1;
            numBytesReceived--;
        }

        memcpy((fileBuffer + fileBufferTracker), receiveBuffer, numBytesReceived);
        fileBufferTracker += numBytesReceived;
        numPacketsReceived++;

        if (numPacketsReceived == BATCH_SIZE || isLastPacket) {
            numPacketsReceived = 0;

            // send ACK to Client
            acksPacket.sequenceNumber = 1;
            acksPacket.packetLength = 0;

            if ((numBytesSent = sendto(sockfd, &acksPacket, sizeof(acksPacket), 0, (struct sockaddr*) &client_addr, clientStructLength)) == -1) {
                printf("Error sending ACK from Server to Client");
                exit(1);
            }
        }
    }


    if ((fp = fopen("myUDPReceive.txt", "wt")) == NULL)
    {
        printf("File doesn't exist\n");
        exit(0);
    }

    fwrite(fileBuffer, 1, fileBufferTracker, fp);
    fclose(fp);
    printf("File from Client has been successfully received!\nThe total EFFECTIVE data received is %d bytes\n", (int)fileBufferTracker);


    // ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t * addrlen)
    // sockfd    = Socket to receive through
    // buf       = buffer containing data to be received
    // len       = size of buffer containing data to be received
    // flags     = Bitwise OR flags to modify Socket behaviour
    // src_addr  = Structure containing source address is RETURNED
    // src_len   = size of source address is RETURNED
    // can respond to Client by sending to client_addr
}
