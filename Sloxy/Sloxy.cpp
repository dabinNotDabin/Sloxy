#include "Sloxy.h"


Sloxy::Sloxy()
{

}

Sloxy::~Sloxy()
{

}


// Takes an HTTP request and deciphers the host's information.
// Sets hostName to the human readable host name and hostNameLen to the length of the host's name.
// Sets port = 80 if no port number specified, otherwise sets port equal to the specified port number.
void Sloxy::getHostInfoFromRequest(char *httpRequest, char *hostName, int &hostNameLen, int &port)
{
	string req(httpRequest);

	// Find the "Host:" tag
	size_t pos = req.find("Host:");
	size_t end;
	int len;

	// Ensures that the request contained the "Host:" tag
	if (pos != string::npos)
	{
		pos += 6;
		end = pos;

		// Find the end of the line.
		while (end < req.length() && req[end] != ':' && req[end] != '\r')
			end++;

		// Extract the host name from the request.
		if (end < req.length())
		{
			cout << "Host info (extracted from request):\n";

			len = end - pos;
			string hostStr = req.substr(pos, len);

			cout << "\tName: " << hostStr.c_str() << endl;

			memcpy(hostName, hostStr.c_str(), len + 1);

			hostNameLen = len;
		}
		else
			cout << "Error reading host name, end of request string reached before name extracted.\n";

		// If a port number was specified, extract it from the request.
		if (req[end] == ':')
		{
			end++;
			pos = end;

			while (req[end] != '\r')
				end++;

			len = end - pos;
			string portStr = req.substr(pos, len);

			port = atoi(portStr.c_str());
		}
		else
			port = 80;

		cout << "\tPort requested: " << port << endl;
	}
	else
		cout << "Host: tag not found in request header.\n";
}



void Sloxy::receiveMessage(int socketID, char message[], int &msgLength)
{
	int receivedCount = -1;
	int errsv = 0;
	char rcv_message[10000];

	receivedCount = recv(socketID, rcv_message, 10000, 0);
	if (receivedCount == 0)
	{
		cout << "No messages and peer has shutdown connection..\n";
		return;
	}
	else if (receivedCount == -1)
	{
		errsv = errno;
		cout << "Receive message failed..\n";
		// process errsv if necessary
		return;
	}
	else
	{
		memcpy(message, rcv_message, receivedCount + 1);
		msgLength = receivedCount;
	}
}

void Sloxy::sendMessage(int socketID, char message[], int msgLength)
{
	int sentCount = -1;
	int errsv = 0;

	sentCount = send(socketID, message, msgLength, 0);
	//sentCount = send(webServerSocketID, headRequest, receivedCount+1, 0);
	while (sentCount == -1 || sentCount < msgLength)
	{
		errsv = errno;
		cout << "Send message failed or incomplete.\n";
		// process errsv if necessary
	}

	cout << "Sent to web server: " << sentCount << endl;
}


// Uses an http request message to build a host internet address
void Sloxy::buildHostInetAddr(string hostName, int port, struct sockaddr_in &hostInetAddress)
{
	struct hostent *hostEnt = NULL;
	// if string arg is already a valid IP, no lookup is performed
	hostEnt = gethostbyname(hostName.c_str());

	if (hostEnt == NULL)
	{
		cout << "Host name not found.\n";
		return;
	}
	else
		cout << "Host name to IP lookup successful.\n";

	struct in_addr **addr_list;
	addr_list = (struct in_addr **) hostEnt->h_addr_list;

	memset((&hostInetAddress), 0, sizeof(hostInetAddress));
	hostInetAddress.sin_family = hostEnt->h_addrtype;
	hostInetAddress.sin_port = htons(port);
	hostInetAddress.sin_addr = *addr_list[0];

	cout << "Host IP: " << inet_ntoa(hostInetAddress.sin_addr) << endl;
	cout << "Host address type: " << hostInetAddress.sin_family << endl;
	cout << "Host port: " << ntohs(hostInetAddress.sin_port) << endl;
}




bool Sloxy::isHtml(char *httpRequest)
{
	string req(httpRequest);

	size_t pos;
	size_t end;
	int len;

	// Isolate the URL.
	pos = req.find(' ') + 1;
	end = req.find(' ', pos);
	len = end - pos;
	string urlString = req.substr(pos, len);


	// Find the last period to isolate the url suffix.
	pos = urlString.find_last_of('.');

	// Find the html suffix.
	end = urlString.find("html", pos);

	// If not found, return false, otherwise, return true.
	if (end == string::npos)
		return false;

	return true;
}


// Changes a GET request to a HEAD request.
// More specifically, replaces the first 3 characters in httpRequest with "HEAD"
void Sloxy::fromGetToHead(char *httpRequest)
{
	string req(httpRequest);
	string headRequest("HEAD" + req.substr(3));

	memcpy(httpRequest, headRequest.c_str(), headRequest.length() + 1);
}


int Sloxy::getContentLength(char * httpResponse)
{
	string req(httpResponse);

	size_t pos = req.find("Content-Length:") + 16;
	size_t end = pos;

	int len;
	int numBytes = -1;

	while (req[end] != '\r')
		end++;

	len = end - pos;
	numBytes = atoi((req.substr(pos, len)).c_str());


	return numBytes;
}


bool Sloxy::acceptsRanges(char *httpResponse)
{
	string req(httpResponse);

	size_t pos = req.find("Accept-Ranges:");
	size_t end;
	int len;

	if (pos == string::npos)
	{
		cout << "\"Accept-Ranges:\" tag not found.\n";
		return false;
	}

	pos += 15;
	end = pos;

	while (req[end] != '\r')
		end++;

	len = end - pos;

	string rangeUnits = req.substr(pos, len);

	if (rangeUnits.compare("bytes") == 0)
		return true;

	return false;
}





void Sloxy::interceptActivity(int port)
{
	int maxConnectQueue = 5;

	struct sockaddr_in hostInternetAddress;

	cout << "Initializing Server.\n";
	server.listenForClients(port, maxConnectQueue);
	server.acceptClientConnection();
	cout << endl;

	string messageContainer;
	string partialMessage;

	char rcv_message[10000];
	char snd_message[10000];
	char headRequest[10000];
	int receivedCount = -1;
	int sentCount = -1;
	int errsv = 0;


	if (1)
	{
		receiveMessage(server.getWebClientSocketID(), rcv_message, receivedCount);

		cout << "Server received a message.\n";
		cout << "---------- START MESSAGE ----------\n";
		for (int i = 0; i < receivedCount; i++)
			cout << rcv_message[i];
		cout << "----------- END MESSAGE -----------\n";

		// ========== GET WEB SERVER ADDRESS FROM REQUEST ==========
		char * hostName = new char[200];
		int hostNameLen = 0;
		int webServerPort;

		cout << "\nGathering host information from request by client.\n";

		getHostInfoFromRequest(rcv_message, hostName, hostNameLen, webServerPort);
		string hostStr(hostName);

		// If host has not been connected with by an existing representative client
		if (connectedHosts.find(hostStr) == connectedHosts.end() || true)
		{
			buildHostInetAddr(hostStr, webServerPort, hostInternetAddress);

			cout << "\nConnecting to host.\n";

			client.connectWithHost(hostInternetAddress);

			memcpy(headRequest, rcv_message, receivedCount + 1);
			fromGetToHead(headRequest);

			memcpy(snd_message, rcv_message, receivedCount + 1);

			sendMessage(client.getWebHostSocketID(), snd_message, receivedCount);

			cout << "Message of length " << sentCount << " sent to web server.\n";

			memset(rcv_message, 'x', 10000);
			receiveMessage(client.getWebHostSocketID(), rcv_message, receivedCount);

			cout << "\nMessage received from web server.\n";
			cout << "---------- START MESSAGE ----------\n";
			for (int i = 0; i < receivedCount; i++)
				cout << rcv_message[i];
			cout << "----------- END MESSAGE -----------\n";

			sentCount = send(server.getWebClientSocketID(), rcv_message, receivedCount, 0);
			if (sentCount == -1 || sentCount != receivedCount)
			{
				errsv = errno;
				cout << "Send message failed or incomplete.\n";
				// process errsv if necessary
			}
			else
			{
				cout << "\nMessage sent back to web client.\n\n\n";
			}
		}
		else
		{
			// a client has already connected to this host.
			// do multithreading here.
		}

/*
		if (isHtml(rcv_message))
		{
			memcpy(headRequest, rcv_message, receivedCount + 1);
			fromGetToHead(headRequest);

			cout << "\nTransformed HEAD request: \n\n";
			cout << "---------- START MESSAGE ----------\n";
			for (int i = 0; i < receivedCount + 1; i++)
				cout << headRequest[i];
			cout << "----------- END MESSAGE -----------\n\n";



		}
		else
		{
			// relay directly to host
		}
*/
	}

	return;
}
