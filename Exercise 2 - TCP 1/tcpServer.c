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
    struct sockaddr_in client_addr;    // Server will respond to the client at this address
    int client_size;

    // Create Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <0)
    {
        printf("Error in socket!");
        exit(1);
    }

    // Filling in Server Address paramters (Server receives from this address)
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(MYTCP_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero(&(server_addr.sin_zero), 8);

    // Associate Socket to some Port, so Server knows which IP-Address/Port pair it should listen to
    // e.g. have Server Socket be bound to Port 80 for Web Server
    ret = bind(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr));
    if (ret < 0)
    {
        printf("Error in binding");
        exit(1);
    }

    // listen() converts UNCONNECTED socket into PASSIVE socket, indicating that Kernel should accept incoming connection requests directed to this socket
    ret = listen(sockfd, BACKLOG);
    if (ret < 0) {
        printf("Error in listening");
        exit(1);
    }

    printf("Receiving start\n");

    while (1)
    {
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
    struct dataPacket recvs;
    struct ackPacket ack; ack.sequenceNumber = 0;
    FILE *fp;
    char buffer[BUFSIZE];          // Server-side buffer 
    int bytesReceived;             // length of message from Client
    int receivedPacketLength = 1;  // will be less than bytesReceived, since we +HEADLEN to packet in Client

    // never assume that the message you've received is the "entire message"
    // simplifying assumption that we are able to receive and store entire message from ONE send (from Client)
    if ((bytesReceived = recv(sockfd, &recvs, MAXSIZE, 0)) == -1)    // receive from Client (has additional HEADLEN size)
    {
        printf("Receiving error!\n");
        return;
    }
    else {
        printf("%d bytes of data received\n", bytesReceived);
    }

    receivedPacketLength = recvs.packetLength;                        // packetLength < bytesReceived (as the size of transmission was increased on Client side due to +HEADLEN)
    memcpy(buffer, recvs.packetData, (bytesReceived - HEADLEN));      

    // send ACK (these parameters represent ACK, feel free to change)
    ack.packetLength = 0;
    ack.sequenceNumber = 1;
    send(sockfd, &ack, 2, 0);

    // Write from buffer into a .txt file
    if((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
    {
        printf("File doesn't exit\n");
        exit(0);
    }

    // data received from Client
    printf("The data received in bytes: %d\n", bytesReceived);
    printf("The file size received in bytes: %d\n", receivedPacketLength);
    fwrite (buffer , 1 , receivedPacketLength, fp);
    fclose(fp);
    printf("A file has been successfully received!\n");
}
