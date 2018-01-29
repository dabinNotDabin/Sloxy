#include "Sloxy.h"
#include "Client.h"



Client::Client()
{

}

Client::~Client()
{

}


bool Client::connectWithHost(struct sockaddr_in hostAddress)
{
	// Need an init that takes more info to set the socket up according to the host's protocol
	if (clientSocket.init())
	{
		if (!clientSocket.connectToHost(hostAddress))
		{
			cout << "Connect to host failed.\n";
			return false;
		}
	}
	else
	{
		cout << "Instantiation of socket failed.\n";
	}

	return true;
}


