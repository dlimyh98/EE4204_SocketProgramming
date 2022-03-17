/*******************************
 * TCP Client (Data is sent from here)
 * 1. Create Socket
 * 2. Connect
 * 3. Send
 * 
 * TCP is a connection-oriented protocol. It requires connection to be ESTABLISHED between two processes before data can be exchanged.
 ********************************/
#include "headsock.h"
void sendStringOnClient(FILE *fp, int sockfd);         

int main(int argc, char **argv)
{
    int sockfd, ret;
    struct sockaddr_in ser_addr;    // Client will send data to this server address
    struct hostent *userHostEnt;    // User inputted hostent
    struct in_addr **userAddress;

    /************************************************** Parsing Input *****************************************************************/
    if (argc != 2) {
        printf("parameters not match");
    }

    userHostEnt = gethostbyname(argv[1]);	         
    if (userHostEnt == NULL) {
        printf("Error when gethostby name");
        exit(0);
    }

    /**************************************** Printing Host Information (optional) *********************************************/
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

    /************************************************** Create Socket *****************************************************************/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);    // TCP - Data is in the form of Byte Stream
    if (sockfd <0)
    {
        printf("Error in socket");
        exit(1);
    }

    /*********************************** Updating Address Parameters of Server Address *************************************************/
    char** addressList = userHostEnt->h_addr_list;       // NULL-terminated array of in_addr structs
    userAddress = (struct in_addr**)addressList;         // cast to (struct in_addr**)

    ser_addr.sin_family = AF_INET;                                                      
    ser_addr.sin_port = htons(MYTCP_PORT);
    memcpy(&(ser_addr.sin_addr.s_addr), *userAddress, sizeof(struct in_addr));    // Writing Internet Address that User provided
    bzero(&(ser_addr.sin_zero), 8);


    /*********************************** Client Address makes connection request to Socket (NOT IN UDP) *************************************************/
    // int connect(int sockfd, struct sockaddr* serv_addr, int addrlen);
    // sockfd    = Socket we have just created
    // serv_addr = Contains DESTINATION Port and IP Address
    // addrlen   = length in bytes of Server Address structure

    // connect the Socket to the address of the Server (Kernel does bind() here implicitly as well)
    // On the Server's side, it is waiting for this connection at accept()
    ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));
    if (ret != 0) {
        printf ("Connection failed\n"); 
        close(sockfd); 
        exit(1);
    }
    // assert that connection is made

    /*********************************** Perform Transmission *************************************************/
    sendStringOnClient(stdin, sockfd);    
    close(sockfd);
    exit(0);
}

void sendStringOnClient(FILE *fp, int sockfd)
{
    char sends[MAXSIZE];

    printf("Please input a string (less than 50 character):\n");
    if (fgets(sends, MAXSIZE, fp) == NULL) {
        printf("Error input\n");
    }

    // TCP no need address parameters (because we already have a communication channel, and only that RELIABLE path should be used for communication)
    send(sockfd, sends, strlen(sends), 0);
    printf("Send out!!\n");

    // Client can receive from Server as well
    char server_message[MAXSIZE];
    if (recv(sockfd, server_message, sizeof(server_message), 0) < 0) {
        printf("Error in receiving Server's response\n");
        // return -1;
    }
    printf("Server's Response is %s\n", server_message);
}

