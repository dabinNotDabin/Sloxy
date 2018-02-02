#include "Sloxy.h"
#include "Server.h"

using namespace std;


bool isHtml(char *httpRequest);
void fromGetToHead(char *httpRequest);
int getContentLength(char *httpResponse);
bool acceptsRanges(char *httpResponse);




int main(int argc, char * argv[])
{
	Sloxy sloxy;
	int port = 80;

	if (argc > 1)
		port = atoi(argv[1]);

	
	sloxy.interceptActivity(port);



	return 0;
}








Server::Server()
{
	webClientID = -1;
}


Server::~Server()
{
	close(webClientID);
}

void Server::listenForClients(int port, int maxConnectQue)
{
	if (listenSocket.init())
	{
		listenSocket.setAddress(port);
		if (listenSocket.bindAddressWithSocket())
		{
			if (!listenSocket.listenToSocket(maxConnectQue))
			{
				cout << "Failed to listen to socket.\n";
			}
		}
		else
		{
			cout << "Binding address to socket with port: " << port << " failed.\n";
		}
	}
	else
	{
		cout << "Instantiation of socket failed.\n";
	}
}


// Accepts a connection through a previously configured listener socket.
// If successful, the webClientID will be set as part of the server's state.
void Server::acceptClientConnection()
{
	//int tempID;

	//tempID = accept(listenSocket.getID(), NULL, NULL);
	//if (tempID == -1)
	//{
	//	cout << "Accept connection failed..\n";
	//	return;
	//}

	//webClientID = tempID;

	listenSocket.acceptClientConnection(webClientID);
}

int Server::getWebClientSocketID()
{
	return webClientID;
}


int Server::getListenerSocketID()
{
	return listenSocket.getID();
}
