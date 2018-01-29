#include "Sloxy.h"
#include "Socket.h"

using namespace std;

Socket::Socket()
{
	addressInitialized = false;
}

Socket::~Socket()
{
	close(ID);
}


int Socket::getID()
{
	return ID;
}

bool Socket::init()
{
	ID = socket(AF_INET, SOCK_STREAM, 0);

	if (ID < 0)
	{
		cout << "Socket creation failed.. Socket id is -1.\n";
		return false;
	}

	cout << "Server socket initialized\n";

	return true;
}

void Socket::setAddress(int port)
{
	memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(port);
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	addressInitialized = true;
}

// Assigns the 
bool Socket::bindAddressWithSocket()
{
	if (ID < 0 || !addressInitialized)
	{
		if (ID < 0)
			cout << "Socket ID not valid.\n";
		else
			cout << "Socket address must be initialzed with a call to \"setAddress(2)\" before binding.\n";

		return false;
	}


	// Bind the socket to the port
	int bindResult = bind(ID, (struct sockaddr *)&socketAddress, sizeof(socketAddress));
	if (bindResult  < 0)
	{
		cout << "Socket binding failed.\n";
		return false;
	}
	
	cout << "Address bound to socket.\n";

	return true;
}

bool Socket::listenToSocket(int connectQueueMax)
{
	if (ID < 0)
	{
		cout << "The socket ID is not valid. It must be instantiated with a call to \"init()\".\n";
		return false;
	}


	int listenResult = listen(ID, connectQueueMax);
	if (listenResult < 0)
	{
		cout << "Socket listen failed..\n";
		return false;
	}

	cout << "Server socket listening...\n";

	return true;
}


// Accept a connection request.
// This will block until a connection is accepted.
// Will not change clientSocketID unless a connection is accepted successfully.
bool Socket::acceptClientConnection(int &clientSocketID)
{
	int tempID = -1;

	if (ID < 0)
	{
		cout << "The socket ID is not valid. It must be instantiated with a call to \"init()\".\n";
		return false;
	}

	cout << "Waiting for a client to request a connection...\n";
	tempID = accept(ID, NULL, NULL);
	if (tempID < 0)
	{
		cout << "Accept connection failed..\n";
		return false;
	}

	clientSocketID = tempID;

	cout << "A connection has been accepted.\n";
	cout << "Ready to receive messages.\n";
	cout << "AF_INET: " << AF_INET << endl;

	return true;
}


bool Socket::connectToHost(struct sockaddr_in hostAddress)
{
	if (ID < 0)
	{
		cout << "Socket ID not valid.\n";
		return false;
	}

	// Connect to the web server.
	int connectResult = connect(ID, (struct sockaddr *)&hostAddress, sizeof(hostAddress));
	if (connectResult == -1)
	{
		cout << "Socket connect failed..\n";
		return false;
	}

	return true;
}