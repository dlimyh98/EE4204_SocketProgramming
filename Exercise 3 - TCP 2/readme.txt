This example shows how to transmit a large file using small packets (individual packet size is fixed at 100bytes). 
Server transmits the acknowledgement to Client when the last byte is received. 
In test, the file size is 50554 bytes.

1. in Exercise 2, we were packing the data to be transmitted INSIDE the dataPacket.packetData[MAXLEN] field
2. in Exercise 3, we are directly transmitting a packet via char sendPacket[DATALEN]. Only ACK is packed inside a struct and then sent

QED. You can transmit information (Client to Server bidirectional) through different means