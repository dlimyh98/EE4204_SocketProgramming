/**************************************
* UDP Server (data is received here)
* 1. Create UDP Socket (socket())
* 2. Associate socket with some port on local machine (bind())
* 3. Wait until datagram arrives from the Client (recvfrom())
* 4. Process the datagram packet and send response to Client (not done here)
* 5. Repeat from step 3
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
	while(1) {
		receiveStringOnServer(sockfd, client_addr);
	}
	close(sockfd);
	exit(0);
}

void receiveStringOnServer(int sockfd, struct sockaddr_in client_addr)
{
    // Unlike TCP, Server does not wait for Client to connect
    // Therefore, Server does not receive Client's address prior to sending or receiving data (yes, Server can send data to Client!)
    // Instead, Server receives information about Client when it receives data from Client (from recv())
	char recvs[MAXSIZE];
    int n = 0;
    int clientStructLength = sizeof(client_addr);

    // ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t * addrlen)
    // sockfd    = Socket to receive through
    // buf       = buffer containing data to be received
    // len       = size of buffer containing data to be received
    // flags     = Bitwise OR flags to modify Socket behaviour
    // src_addr  = Structure containing source address is RETURNED
    // src_len   = size of source address is RETURNED

	if ((n=recvfrom(sockfd, &recvs, MAXSIZE, 0, (struct sockaddr* )&client_addr, &clientStructLength)) == -1) {
		printf("error receiving");
		exit(1);
	}
	recvs[n] = '\0';
	printf("The received string is : %s", recvs);

    // can respond to Client by sending to client_addr
}
