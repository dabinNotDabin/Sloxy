#pragma once
#include <vector>
#include <map>

#include "Socket.h"

using namespace std;


// In this implementation, a client will connect to one host.
// The sloxy has the responsibility of managing the various representative clients.
class Client
{
public:
	Client();
	~Client();

	bool connectWithHost(struct sockaddr_in hostAddress);


private:
	// Client socket used to ask web server for information.
	// It provides an api that allows data to be sent to and received from that web server.
	Socket clientSocket;


};