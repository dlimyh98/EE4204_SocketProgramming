/*******************************
* UDP Client (data is transmitted from here)
* 1. Create a UDP Socket (socket())
* 2. Send message to server (sendto())
* 3. Wait until response from Server is received
* 4. Process reply and repeat from step 2 (not done here)
* 
* In theory, UDP Client does not need to form connection with Server (like in TCP), it can just send a datagram (UDP has no acknowledgements).
* Similarly, the UDP Server does not need to accept a connection, it can simply wait for datagrams to arrive.
* Datagrams upon arrival contain the address of the Client, which the Server uses to respond back to the correct Client.
* every UDP datagram is also STAND-ALONE, unlike TCP which comes in a STREAM
********************************/
#include "headsock.h"
void sendStringOnClient(FILE *fp, int sockfd, struct sockaddr* serverAddress, int serverAddressLength);                


int main(int argc, char *argv[])
{
    /************************************************** Variable Declarations *****************************************************************/
    // struct sockaddr_in holds...
    // 1. IP Address in sin_addr member (which is of type struct in_addr)
    // 2. Port Number in sin_port member
    // struct sockaddr_in is a base of a set of address structure that act like discriminated union. For our purposes, it acts like IPv4 address
	struct sockaddr_in server_address;    // Client will send to this address

	int len, sockfd;
	struct hostent* userHostEnt;          // struct hostent is used by functions to store information about a given host (can be Client or Server)
	struct in_addr** userServerAddress;   // struct in_addr represents IPv4 Internet Address

    /************************************************** Parsing Input *****************************************************************/
    // Verify that enough arguments are given
	if (argc!= 2)
	{
		printf("parameters not match.");
		exit(0);
	}

    // OLD METHOD
    // 1. gethostbyname() returns a POINTER to the hostent structure (according to User input), AKA returns name of computer that your program is running on
    // 2. load information by hand into a struct sockaddr_in
	if ((userHostEnt=gethostbyname(argv[1])) == NULL) {
		printf("error when gethostbyname");
		exit(0);
	}
    // NEW METHOD - use getaddrinfo(), see Beej's Guide 5.1 for more

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
    // 1. Family - designates the type of Addresses your socket can communicate with (AF_INET) in this case
    // 2. Socket Type  - In what form are you sending your data?
    //                   SOCK_DGRAM for UDP (UDP sends in terms of datagrams)
    //                   UDP is message oriented protocol (data is sent in distinct chunks, the receiver will determine where one message ends and another begins)
    // 3. Protocol Type - 0 (default), leave it up to Service Provider to decide on protocol to be used
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		printf("error in socket");
		exit(1);
	}

    /****************************** Update Parameters for Address that Client will send data to ********************************************/
    char** addressList = userHostEnt->h_addr_list;       // NULL-terminated array of in_addr structs
    userServerAddress = (struct in_addr**)addressList;   // cast to (struct in_addr**)

	server_address.sin_family = AF_INET;                 // AF_INET Family
	server_address.sin_port = htons(MYUDP_PORT);         // Address that client is sending to, will have this UDP_PORT number
    // IP Address                 = Address of system in the Network
    // Port                       = Address of service within the System
    // IP Address + Port (Socket) = Address of PARTICULAR service on PARTICULAR system

    // server_address.sin_addr = IN_ADDR structure that contains Internet Address
    // server_address.sin_addr.s_addr = 4-byte IPv4 address (in Network Byte Order)
    // memcpy FROM *userServerAddress (pointer to in_addr structs, which has USER INPUTTED Address)
    //        TO &(server_address.sin_addr.s_addr), SERVER's IPv4 field
	memcpy(&(server_address.sin_addr.s_addr), *userServerAddress, sizeof(struct in_addr));    // writing Internet Address that User provided (we are going to SEND to this Internet Address)
	bzero(&(server_address.sin_zero), 8);    // for padding purposes

	sendStringOnClient(stdin, sockfd, (struct sockaddr* )&server_address, sizeof(struct sockaddr_in));    // Send, USER-DEFINED function
	close(sockfd);                                                                                        // close Socket once done
	exit(0);
}


void sendStringOnClient(FILE* fp, int sockfd, struct sockaddr* serverAddress, int serverAddressLength)
{
    // MAXSIZE = limited to 50 characters
	char stringToSend[MAXSIZE];
	printf("Please input a string (less than 50 characters):\n");

	if (fgets(stringToSend, MAXSIZE, fp) == NULL) {
		printf("error input\n");
	}

    // Outgoing connections do not need to explicitly bind, only need to bind if Server expects Client to be coming from a specific Port
    // If not explicitly bind, OS will assign a TEMPORARY port number

    // ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen)
    // sockfd    = Socket to send through
    // buf       = buffer containing data to be sent 
    // len       = size of buffer containing data to be sent
    // flags     = Bitwise OR flags to modify Socket behaviour
    // dest_addr = Structure containing address of destination
    // addr_len  = size of dest_addr

    // Unlike TCP, when Client sendTo (or recvsFrom) some Address, the Destination Address has to be specified EACH TIME
	sendto(sockfd, &stringToSend, strlen(stringToSend), 0, serverAddress, serverAddressLength);
	printf("send out!!\n");
}
