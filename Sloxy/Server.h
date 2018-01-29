#pragma once
#include "Socket.h"



// It may be more elegant to maintain a request queue in the server that the sloxy can 
// query and relay to the representative client but since it only needs to service
// one client, I will instead have the Sloxy use the server to receive messages
// and process them as necessary before relaying requests to the representative client.
class Server
{
public:
	Server();
	~Server();

	// This function listens for client connection requests on port <port> with a 
	// maximum client queue length of maxConnectQueue.
	void listenForClients(int port, int maxConnectQue);

	// This function accepts a client connection from the currently configured listener socket.
	void acceptClientConnection();


private:
	// This server only needs to service one client at a time so this will suffice
	int webClientID;

	Socket listenSocket;

	// Server socket only serves data to a client
	// Server socket calls accept with the id from an instantiated and initialized listener socket and obtains a client id.
	// It then provides functions that allow data to be sent to and received from that client.
	Socket serverSocket;



};


