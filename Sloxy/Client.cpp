#include "Sloxy.h"

using namespace std;

int main(int argc, char * argv[])
{
	int clientSocketID;

	// Create a socket ID
	clientSocketID = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocketID == -1)
	{
		cout << "Socket creation failed..\n";
		return 0;
	}

	// Have to connect to the real web server not the server I created.
	// Then relay the GET requests that the web browser sent to my server.
	// Then send the results back to the web browser.
	int connectResult = connect(clientSocketID, (struct sockaddr *)&proxyServerAddress, sizeof(proxyServerAddress));
	if (connectResult == -1)
	{
		cout << "Client Socket connect failed..\n";
		return 0;
	}



	return 0;
}