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


	sloxy.interceptActivity(port);

/*
	int listenerSocketID;
	int webClientSocketID;
	int webServerSocketID;

	struct sockaddr_in proxyServerAddress;

	memset((&proxyServerAddress), 0, sizeof(proxyServerAddress));

	// Create a socket ID -- now done when socket instantiated
	listenerSocketID = socket(AF_INET, SOCK_STREAM, 0);
	if (listenerSocketID == -1)
	{
		cout << "Socket creation failed..\n";
		return 0;
	}

	// Initialize the serverAddress -- done when socket instantiated with port as argument
	proxyServerAddress.sin_family = AF_INET;
	proxyServerAddress.sin_port = htons(port);
	proxyServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	cout << "Server socket initialized\n";

	// Bind the socket to the port
	int bindResult = bind(listenerSocketID, (struct sockaddr *)&proxyServerAddress, sizeof(proxyServerAddress));
	if (bindResult == -1)
	{
		cout << "Socket binding failed..\n";
		return 0;
	}

	cout << "Server socket bound to port " << port << endl;

	// Listen for a connection request
	int listenResult = listen(listenerSocketID, 5);
	if (listenResult == -1)
	{
		cout << "Socket listen failed..\n";
		return 0;
	}

	cout << "Server socket listening...\n";

	


	int receivedCount;
	int sentCount;
	int errsv;
	char rcv_message[10000];
	struct sockaddr_in webServerAddress;

	while (1)
	{
		// Accept a connection request.
		// This will block until a connection is accepted.
		cout << "Waiting for a client to request a connection...\n";
		webClientSocketID = accept(listenerSocketID, NULL, NULL);
		if (webClientSocketID == -1)
		{
			cout << "Accept connection failed..\n";
			return 0;
		}

		cout << "A connection has been accepted.\n";
		cout << "Ready to receive messages.\n";
		cout << "AF_INET: " << AF_INET << endl;


		// ========== RECEIVE MESSAGE FROM WEB CLIENT ==========
		receivedCount = -1;
		errsv = 0;
		receivedCount = recv(webClientSocketID, rcv_message, 10000, 0);
		if (receivedCount == 0)
		{
			cout << "No messages and peer has shutdown connection..\n";
			break;
		}
		else if (receivedCount == -1)
		{
			errsv = errno;
			cout << "Receive message failed..\n";
			// process errsv if necessary
		}
		else
		{
			cout << "\nReceived request from web client of length: " << receivedCount << endl << endl;
			cout << "---------- START MESSAGE ----------\n";
			for (int i = 0; i < receivedCount; i++)
				cout << rcv_message[i];
			cout << "----------- END MESSAGE -----------\n\n";

			// Create HEAD request
			char headRequest[10000];
			memcpy(headRequest, rcv_message, receivedCount + 1);
			fromGetToHead(headRequest);


			cout << "\nTransformed HEAD request: \n\n";
			cout << "---------- START MESSAGE ----------\n";
			for (int i = 0; i < receivedCount + 1; i++)
				cout << headRequest[i];
			cout << "----------- END MESSAGE -----------\n\n";

			
			// ========== GET WEB SERVER ADDRESS FROM REQUEST ==========
			char * host = new char[200];
			int length = 0;
			int webServerPort;

			Sloxy s;
			s.getHostInfoFromRequest(rcv_message, host, length, webServerPort);
			string hostStr(host);
			cout << "Host Extracted: " << hostStr.c_str() << endl;

			struct hostent *hostEnt = NULL;
			// if string arg is already a valid IP, no lookup is performed
			hostEnt = gethostbyname(hostStr.c_str());

			if (hostEnt == NULL)
			{
				cout << "Host name not found.\n";
				break;
			}
			else
				cout << "Host resolved.\n";

			struct in_addr **addr_list;
			addr_list = (struct in_addr **) hostEnt->h_addr_list;

			memset((&webServerAddress), 0, sizeof(webServerAddress));
			webServerAddress.sin_family = hostEnt->h_addrtype;
			webServerAddress.sin_port = htons(webServerPort);
			webServerAddress.sin_addr = *addr_list[0];

			cout << "Web server address: " << inet_ntoa(webServerAddress.sin_addr) << endl;
			cout << "Web server address type: " << webServerAddress.sin_family << endl;
			cout << "Web server address port: " << ntohs(webServerAddress.sin_port) << endl;


			// ========== PROCESS REQUEST ==========
			bool mediaIsHtml = false;
			mediaIsHtml = isHtml(rcv_message);
			if (mediaIsHtml)
				cout << "Requested media is/contains html.\n";
			else
				cout << "Requested media does not contain html.\n";



			// Initialize this next socket in the client class
			// use the web server address extracted by the sloxy for the h_addrtype
			// Have the client connect to the host
			// Check that the media is html
			// Send a head request to determine the content size and to determine if range requests are accepted
			// Request 100 bytes at a time until the content size is reached
			// The client may have to ensure that all 100 bytes are received every request.
			// Append these retrievals and relay to actual client using server object.
			// 


			// ========== CONNECT TO WEB SERVER AND RELAY REQUEST ==========
			// Create a socket ID referencing the connection with the web server.
			webServerSocketID = socket(hostEnt->h_addrtype, SOCK_STREAM, 0);
			if (webServerSocketID == -1)
			{
				cout << "Socket creation failed..\n";
				return 0;
			}

			// Connect to the web server.
			int connectResult = connect(webServerSocketID, (struct sockaddr *)&webServerAddress, sizeof(webServerAddress));
			if (connectResult == -1)
			{
				cout << "Client Socket connect failed..\n";
				break;
			}
			
			sentCount = send(webServerSocketID, rcv_message, receivedCount, 0);
			//sentCount = send(webServerSocketID, headRequest, receivedCount+1, 0);
			if (sentCount == -1 || sentCount != receivedCount)
			{
				errsv = errno;
				cout << "Send message failed or incomplete.\n";
				// process errsv if necessary
			}


			cout << "Received from web client: " << receivedCount << endl;
			cout << "Sent to web server: " << sentCount << endl;


			// ========== RECEIVE RESPONSE FROM WEB SERVER AND RELAY TO WEB CLIENT ==========
			receivedCount = -1;
			errsv = 0;
			memset(rcv_message, 'x', 10000);
			receivedCount = recv(webServerSocketID, rcv_message, 10000, 0);
			if (receivedCount == 0)
			{
				cout << "No messages and peer has shutdown connection..\n";
				break;
			}
			else if (receivedCount == -1)
			{
				errsv = errno;
				cout << "Receive message failed..\n";
				// process errsv if necessary
			}
			else
			{
				cout << "\n\nMessage received from web server.\n";

				int contentLength = getContentLength(rcv_message);
				cout << "Declared Content Length: " << contentLength << endl;

				bool acceptsRangeRequests = false;
				acceptsRangeRequests = acceptsRanges(rcv_message);

				if (acceptsRangeRequests)
					cout << "Web server accepts range requests.\n";
				else
					cout << "Web server doesn't accept range requests.\n";


				cout << "\nReceived Count: " << receivedCount << endl << endl;
				cout << "---------- START MESSAGE ----------\n";
				for (int i = 0; i < receivedCount; i++)
					cout << rcv_message[i];
				cout << "----------- END MESSAGE -----------\n";


				if (receivedCount < contentLength)
				{
					receivedCount = recv(webServerSocketID, rcv_message, 10000, 0);

					cout << "\nReceived Count: " << receivedCount << endl << endl;
					cout << "---------- START MESSAGE ----------\n";
					for (int i = 0; i < receivedCount; i++)
						cout << rcv_message[i];
					cout << "----------- END MESSAGE -----------\n";
				}

				sentCount = send(webClientSocketID, rcv_message, receivedCount, 0);
				if (sentCount == -1 || sentCount != receivedCount)
				{
					errsv = errno;
					cout << "Send message failed or incomplete.\n";
					// process errsv if necessary
				}
				else
				{
					cout << "Message sent back to web client.\n\n\n";
				}
			}

			delete[] host;
		}
	}


	close(listenerSocketID);
	close(webClientSocketID);
	close(webServerSocketID);

	*/

	return 0;
}








Server::Server()
{
	webClientID = -1;
}


Server::~Server()
{

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