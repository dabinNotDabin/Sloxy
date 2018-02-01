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




void Sloxy::sendMessage(int socketID, char message[], int msgLength, int &bytesSent)
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

	bytesSent = sentCount;
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


int Sloxy::getHttpResponseCode(char *httpResponse)
{	
	size_t posA = -1;
	size_t posB = -1;
	string response(httpResponse);

	posA = response.find_first_of(' ');

	if (posA == string::npos)
	{
		return -1;
	}
	else
	{
		posB = response.find_first_of(' ', posA + 1);
		if (posB == string::npos)
		{
			return -1;
		}
		else
		{
			posA++;
			return atoi((response.substr(posA, posB - posA)).c_str());
		}
	}
}



// Changes a GET request to a HEAD request.
// More specifically, replaces the first 3 characters in httpRequest with "HEAD"
void Sloxy::fromGetToHead(char *httpRequest, int &msgLength)
{
	string req(httpRequest);
	string headRequest("HEAD" + req.substr(3));

	memcpy(httpRequest, headRequest.c_str(), headRequest.length() + 1);

	msgLength = headRequest.length();
}



// Changes a GET request to a HEAD request.
// More specifically, replaces the first 3 characters in httpRequest with "HEAD"
void Sloxy::fromGetToRangeGet(char *httpRequest, int beginByte, int numBytes, int &msgLength)
{
	char buffer[20];

	string req(httpRequest);
	string begin = to_string(beginByte);
	string end = to_string(beginByte + numBytes - 1);

	req = req.substr(0, req.length() - 2);
	string rangeRequest(req + "Range: bytes=" + begin + "-" + end + "\r\n\r\n");

	memcpy(httpRequest, rangeRequest.c_str(), rangeRequest.length() + 1);

	msgLength = rangeRequest.length();
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

	string messageBuilder;
	string partialMessage;
	string header;

	char fromClientBuffer[10000];
	char toClientBuffer[10000];

	char fromHostBuffer[10000];
	char toHostBuffer[10000];

	char headRequest[10000];
	char rangeRequest[10000];

	int bytesRcvdFromClient = -1;
	int bytesSentToClient = -1;
	int bytesRcvdFromHost = -1;
	int bytesSentToHost = -1;
	int headerLength = -1;
	int errsv = 0;
	int contentLength = -1;
	int bytesReceived;

	int totalMsgLength;
	int totalReceived;

	int rangeSize = 100;
	int bytesToGet = -1;
	int msgLen = -1;

	if (1)
	{
		receiveMessage(server.getWebClientSocketID(), fromClientBuffer, bytesRcvdFromClient);

		cout << "Server received a message of length: " << bytesRcvdFromClient << endl;
		cout << "---------- START MESSAGE ----------\n";
		for (int i = 0; i < bytesRcvdFromClient; i++)
			cout << fromClientBuffer[i];
		cout << "----------- END MESSAGE -----------\n";

		// ========== GET WEB SERVER ADDRESS FROM REQUEST ==========
		char * hostName = new char[200];
		int hostNameLen = 0;
		int webServerPort;

		cout << "\nGathering host information from request by client.\n";

		getHostInfoFromRequest(fromClientBuffer, hostName, hostNameLen, webServerPort);
		string hostStr(hostName);

		// If host has not been connected with by an existing representative client
		if (connectedHosts.find(hostStr) == connectedHosts.end() || true)
		{
			buildHostInetAddr(hostStr, webServerPort, hostInternetAddress);

			cout << "\nConnecting to host.\n";

			client.connectWithHost(hostInternetAddress);
		}
		else
		{
			// a client has already connected to this host.
			// do multithreading here.
		}


		if (isHtml(fromClientBuffer))
		{
			memcpy(headRequest, fromClientBuffer, bytesRcvdFromClient + 1);
			memcpy(toHostBuffer, fromClientBuffer, bytesRcvdFromClient + 1);

			fromGetToHead(headRequest, msgLen);
			sendMessage(client.getWebHostSocketID(), headRequest, msgLen, bytesSentToClient);

			cout << "Head request of length " << bytesSentToClient << " sent to web server.\n";

			memset(fromHostBuffer, 'x', 10000);
			receiveMessage(client.getWebHostSocketID(), fromHostBuffer, bytesRcvdFromHost);

			cout << "The header that the web server replied with had length: " << bytesRcvdFromHost << endl;

			contentLength = getContentLength(fromHostBuffer);

			totalMsgLength = bytesRcvdFromHost + contentLength;
			cout << "The header size plus the content length should be: " << totalMsgLength << endl;

			headerLength = bytesRcvdFromHost;
			header.assign(fromHostBuffer, headerLength);

			cout << "\n\nHeader:\n\n" << header;

			cout << "HTTP response: " << getHttpResponseCode(fromHostBuffer) << endl;

			if (acceptsRanges(fromHostBuffer))
			{
				messageBuilder.clear();
				cout << "Web host accepts range requests.\n";
				totalReceived = 0;
				bytesToGet = rangeSize;

				while (totalReceived < contentLength)
				{
					if ((contentLength - totalReceived) < rangeSize)
						bytesToGet = contentLength - totalReceived;

					memcpy(rangeRequest, toHostBuffer, bytesRcvdFromClient + 1);
					fromGetToRangeGet(rangeRequest, totalReceived, bytesToGet, msgLen);

					cout << "\nSending message to web host of length: " << msgLen << endl;
					cout << "---------- START MESSAGE ----------\n";
					for (int i = 0; i < msgLen; i++)
						cout << rangeRequest[i];
					cout << "----------- END MESSAGE -----------\n";

					client.connectWithHost(hostInternetAddress);
					sendMessage(client.getWebHostSocketID(), rangeRequest, msgLen, bytesSentToHost);

					if (bytesSentToHost != msgLen)
						cout << "Full message not sent to web server.\n";

					receiveMessage(client.getWebHostSocketID(), fromHostBuffer, bytesRcvdFromHost);

					cout << "\nReceived message from web host of length: " << bytesRcvdFromHost << endl;
					cout << "---------- START MESSAGE ----------\n";
					for (int i = 0; i < bytesRcvdFromHost; i++)
						cout << fromHostBuffer[i];
					cout << "----------- END MESSAGE -----------\n";


					cout << "\nBytes to Get: " << bytesToGet << endl;

					partialMessage.assign(fromHostBuffer, bytesRcvdFromHost - bytesToGet, bytesRcvdFromHost);
					messageBuilder += partialMessage;

					cout << "\n\n\n\nPartial Message:\n\n" << partialMessage << endl << endl;

					bytesReceived = getContentLength(fromHostBuffer);
					totalReceived += bytesReceived;

					cout << "\nTotal received: " << totalReceived << endl;
					cout << "Message so far: \n" << messageBuilder << endl;
				}

				cout << "Full message received.\n";
			}
			else
			{
				cout << "Host doesn't accept range requests or accepted range tag was missing.\n";
			}

			messageBuilder = header + messageBuilder;

			memcpy(toClientBuffer, messageBuilder.c_str(), totalMsgLength);
			sendMessage(server.getWebClientSocketID(), toClientBuffer, totalMsgLength, bytesSentToClient);
		}
		else
		{
			cout << "File was not html.\n";

			sendMessage(client.getWebHostSocketID(), fromClientBuffer, bytesRcvdFromClient, bytesSentToHost);
			receiveMessage(client.getWebHostSocketID(), fromHostBuffer, bytesRcvdFromHost);
			sendMessage(server.getWebClientSocketID(), fromHostBuffer, bytesRcvdFromHost, bytesSentToClient);
		}

		cout << "\nMessage sent back to web client.\n\n\n";
	}

	return;
}
