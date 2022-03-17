TCP Program for transferring a large message.
 - The message transmitted from Client to Server is read from a large file, and the ENTIRE message is sent by the Client as a stream of bytes.
 - After receiving the file, Server sends an ACK to the Client.
 - We can verify that the file has been sent completely, by using "diff" command between original file and received file

The received data is stored in file "myTCPreceive.txt".

Simplifying Assumptions
1. Able to send() the entire buffer in one go,  i.e. file can be sent through ONE call to send()
2. Able to recv() the entire message in one go, i.e. able to store the file received into serverBuffer in ONE recv() call
