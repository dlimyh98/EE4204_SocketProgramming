/**********************************
 * TCP Server
 ***********************************/
#include "headsock.h"
#define BACKLOG 10

void serverFunc(int sockfd);

int main(void)
{
    int sockfd, ret;
    struct sockaddr_in server_addr;    // Server will receive from this address
    struct sockaddr_in client_addr;    // Server will respond to Client at this address
    int client_size;

    // Create Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <0)
    {
        printf("error in socket!");
        exit(1);
    }

    // Filling in Server Address parameters (Server receives from this Address)
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(MYTCP_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(server_addr.sin_zero), 8);

    // Associate Socket to some Port, so Server knows which IP-Address/Port pair it should listen to
    // e.g. have Server Socket be bound to Port 80 for Web Server
    ret = bind(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr));
    if (ret <0)
    {
        printf("Error in binding");
        exit(1);
    }

    // listen() converts UNCONNECTED socket into PASSIVE socket, indicating that Kernel should accept incoming connection requests directed to this socket
    ret = listen(sockfd, BACKLOG);
    if (ret <0) {
        printf("Error in listening");
        exit(1);
    }

    while (1)
    {
        printf("Waiting for data\n");

        // Store the client's Address and Socket by accepting an incoming connection
        client_size = sizeof (client_addr);
        int clientSocket = accept(sockfd, (struct sockaddr *)&client_addr, &client_size);

        if (clientSocket < 0)
        {
            printf("Error in accepting connection\n");
            exit(1);
        }

        // Create a new process for this Client
        // fork() returns TWICE
        // 1. in the original caller (parent process), with the PID of the newly created child
        // 2. in the newly created child, with 0
        if (fork() == 0)
        {
            // Child Process running (runs concurrently with Parent Process)
            close(sockfd);
            serverFunc(clientSocket);    // receive packet from Client and respond
            close(clientSocket);
            exit(0);
        }
        else {
            // Parent Process running
            close(clientSocket);
        }
    }

    close(sockfd);
    exit(0);
}

void serverFunc(int sockfd)
{
    struct ackPacket acksPacket;
    FILE *fp;
    int isLastPacket = 0;
    char receiveBuffer[DATALEN];      // incoming message from Client is stored here, when recv() is called
    char fileBuffer[BUFSIZE];         // Server side buffer to store ENTIRE file that Client wants to send (NOT just one packet from Client)
    int numBytesReceived = 0;         // Number of bytes Server has received from Client in one recv() call
    int numBytesSent = 0;             // Number of bytes Server has sent to Client in one send() call
    long fileBufferTracker = 0;       // keeps track of where in fileBuffer we should store next packet from Client


    printf("Receiving data!\n");
    while(!isLastPacket)
    {
        if ((numBytesReceived= recv(sockfd, &receiveBuffer, DATALEN, 0)) == -1)
        {
            printf("Error when Server is receiving from Client\n");
            exit(1);
        }

        if (receiveBuffer[numBytesReceived-1] == '\0')      // checking if incoming message from Client will be the last one
        {
            isLastPacket = 1;
            numBytesReceived--;     // since we do need need to copy the '\0' in Client's incoming packet
        }

        memcpy((fileBuffer+fileBufferTracker), receiveBuffer, numBytesReceived);
        fileBufferTracker += numBytesReceived;
    }

    // send ACK to Client
    acksPacket.sequenceNumber = 1;
    acksPacket.packetLength = 0;
    if ((numBytesSent = send(sockfd, &acksPacket, 2, 0)) == -1)
    {
        printf("Error sending ACK from Server to Client");
        exit(1);
    }


    if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
    {
        printf("File doesn't exist\n");
        exit(0);
    }
    fwrite (fileBuffer, 1 , fileBufferTracker , fp);
    fclose(fp);
    printf("A file has been successfully received!\nThe total data received is %d bytes\n", (int)fileBufferTracker);
}
