#pragma once
#include "Socket.h"




class Server
{
public:
	Server();
	~Server();

	void listenForClients(int port, int maxConnectQue);


private:
	int port;


	Socket listenSocket;

	// Client socket asks web server for information.
	// It provides an api that allows data to be sent to and received from that web server.
	// One should be instantiated for every web server the client wants to connect with. (multithreading)
	// 
	Socket clientSocket;

	// Server socket only serves data to a client
	// Server socket calls accept with the id from an instantiated and initialized listener socket and obtains a client id.
	// It then provides functions that allow data to be sent to and received from that client.
	Socket serverSocket;



};


