/**********************************
* TCP Server (Data is received here)
* 1. Create Socket
* 2. Bind
* 3. Listen
* 4. Accept
* 5. Receive
***********************************/

#include "headsock.h"
#define BACKLOG 10

void receiveStringOnServer(int sockfd);     

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
		printf("error in socket!");
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

	printf("Receiving start\n");

	/*while (1)
	{
        // Store the client's Address and Socket by accepting an incoming connection
		client_size = sizeof (client_addr);
		int clientSocket = accept(sockfd, (struct sockaddr *)&client_addr, &client_size); 

		if (clientSocket < 0)
		{
			printf("Error in accept\n");
			exit(1);
		}

        // Create a new process for this Client
        // fork() returns TWICE
        // 1. in the original caller (parent process), with the PID of the newly created child
        // 2. in the newly created child, with 0
		if (fork() == 0)
		{
            // Child Process running (runs concurrently with Parent Process)
			close(sockfd);                         // close the Server Socket (we only transmit one message, so Server Socket won't be needed anymore)
			receiveStringOnServer(clientSocket);   // receive from the Client Socket 
			close(clientSocket);                   
			exit(0);
        }
        else {
            // Parent Process running
            close(clientSocket);
        }
	}
	close(sockfd);
	exit(0);*/

    /********** Alternative Implementation (no fork(), no receiveStringOnServer) ******************/

    // Store the client's Address and Socket by accepting an incoming connection
    client_size = sizeof(client_addr);
    int clientSocket = accept(sockfd, (struct sockaddr*)&client_addr, &client_size);
    if (clientSocket < 0)
    {
        printf("Error in accept\n");
        exit(1);
    }

    // Receive Client's Message
    char clientMessage[MAXSIZE];
    int clientSuccess = recv(clientSocket, clientMessage, sizeof(clientMessage), 0);
    if (clientSuccess < 0) {
        printf("Couldn't receive\n");
        return -1;
    }
    printf("Message from Client: %s\n", clientMessage);

    // Respond to the Client
    char serverResponse[MAXSIZE];
    strcpy(serverResponse, "Hello, I have received your message.\n");
    int serverSuccess = send(clientSocket, serverResponse, sizeof(serverResponse), 0);
    if (serverSuccess < 0) {
        printf("Couldn't send\n");
        return -1;
    }

    close(clientSocket);
    close(sockfd);
}

void receiveStringOnServer(int clientSocket)
{
	char recvs[MAXSIZE];
	int n = 0;

	if ((n= recv(clientSocket, &recvs, MAXSIZE, 0)) == -1) 
	{
		printf("Receiving error!\n");
		return;
	}
	recvs[n] = '\0';
	printf("The received string:\n%s\n", recvs);
}
