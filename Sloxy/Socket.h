#pragma once

/*
Socket class
-


Listener Socket
Client Socket
Server Socket/s
*/

class Socket
{
public:
	Socket();
	~Socket();


	bool init();
	void setAddress(int port);
	bool bindAddressWithSocket();
	bool listenToSocket(int connectQueueMax);
	bool acceptClientConnection(int &clientSocketID);
	int getID();

private:
	// ID assigned on initialization
	int ID;
	bool addressInitialized;

	// Socket address as maintained by OS
	struct sockaddr_in socketAddress;



};


