#include <iostream>
#include <string>
#include <sstream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

int main() {
	WSADATA wsadata;	

	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != 0)
	{
		printf("WSAStartup failed! : %d\n", &wsadata);
		return 1;
	}

	//Create a socket object for the server to listen for the client connection
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListenSocket == SOCKET_ERROR)
	{
		std::cerr << "Cannot create a socket..." << '\n';
		return 1;
	}

	//Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(ListenSocket, (sockaddr*)&hint, sizeof(hint));

	//Tell Winsock that the socket is listening
	listen(ListenSocket, SOMAXCONN);

	//Define set
	fd_set masterSet;

	//Empty the set
	FD_ZERO(&masterSet);
	
	//Add listening sockets to the set
	FD_SET(ListenSocket, &masterSet);

	while (true)
	{
		fd_set copy = masterSet;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == ListenSocket)
			{
				//Accept a new connection
				SOCKET client = accept(ListenSocket, nullptr, nullptr);

				//Add the new connected socket to the list of connected cliencs
				FD_SET(client, &masterSet);

				//Broadcast : we have a new connection
				std::string welcome = "You are connected\n";
				send(client, welcome.c_str(), welcome.size() + 1, 0);
				
			}
			else
			{
				//Create a buffer to receive request
				char buf[1024];
				//Empty the buffer
				ZeroMemory(buf, 1024);

				//Accept a new message
				int bytesReceived = recv(sock, buf, 1024, 0);
				if (bytesReceived <= 0)
				{
					//Drop the client
					closesocket(sock);
					FD_CLR(sock, &masterSet);
				}
				else
				{
					//Send message to other clients, but not the listening sockets
					for (int i = 0; i < masterSet.fd_count; i++)
					{
						SOCKET outSock = masterSet.fd_array[i];
						if (outSock != ListenSocket && outSock != sock)
						{
							std::ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";
							std::string strOut = ss.str();
							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}

	WSACleanup();

	return 0;
}