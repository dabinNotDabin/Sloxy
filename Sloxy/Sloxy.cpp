#include "Sloxy.h"

void receiveMessage(int socketID, char message[], int &msgLength)
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
		message = rcv_message;
		msgLength = receivedCount;
	}
}


// Uses an http request message to build a host internet address
void buildHostInetAddr(char message[], struct sockaddr_in &hostInetAddress)
{
	// ========== GET WEB SERVER ADDRESS FROM REQUEST ==========
	char * hostName = new char[200];
	int hostNameLen = 0;
	int webServerPort;

	getHostInfoFromRequest(message, hostName, hostNameLen, webServerPort);
	string hostStr(hostName);
	cout << "Host Extracted: " << hostStr.c_str() << endl;

	struct hostent *hostEnt = NULL;
	// if string arg is already a valid IP, no lookup is performed
	hostEnt = gethostbyname(hostStr.c_str());

	if (hostEnt == NULL)
	{
		cout << "Host name not found.\n";
		return;
	}
	else
		cout << "Host resolved.\n";

	struct in_addr **addr_list;
	addr_list = (struct in_addr **) hostEnt->h_addr_list;

	memset((&hostInetAddress), 0, sizeof(hostInetAddress));
	hostInetAddress.sin_family = hostEnt->h_addrtype;
	hostInetAddress.sin_port = htons(webServerPort);
	hostInetAddress.sin_addr = *addr_list[0];

	cout << "Web server address: " << inet_ntoa(hostInetAddress.sin_addr) << endl;
	cout << "Web server address type: " << hostInetAddress.sin_family << endl;
	cout << "Web server address port: " << ntohs(hostInetAddress.sin_port) << endl;
}