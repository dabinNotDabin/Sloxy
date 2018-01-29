#pragma once



class Socket
{
public:
	Socket();
	~Socket();
	

	bool init();
	bool init(sin_family family);
	void setAddress(struct sockaddr_in hostAddress);
	void setAddress(int port);
	bool bindAddressWithSocket();
	bool listenToSocket(int connectQueueMax);
	bool acceptClientConnection(int &clientSocketID);
	bool connectToHost(struct sockaddr_in hostAddress);

	int getID();

private:
	// ID assigned on initialization
	int ID;
	bool addressInitialized;

	// Socket address as maintained by OS
	struct sockaddr_in socketAddress;



};


