#include "tcpServer.h"

CtcpServer::CtcpServer(int port, MessageReceivedHandler handler)
	:_port(port), MessageReceived(handler)
{
	FD_ZERO(&_clientList);
}

CtcpServer::~CtcpServer()
{
	cleanUp();
}

void CtcpServer::Send(int senderSocket, int listeningSocket, fd_set &clients, std::string msg)
{
	for (int i = 0;i < _clientList.fd_count;i++)
	{
		SOCKET sock = _clientList.fd_array[i];
		if (sock != senderSocket && sock!=listeningSocket)
		{
			send(sock, msg.c_str(), msg.size() + 1, 0);
		}
	}
}

// Initialize winsock
bool CtcpServer::Init()
{
	WSAData data;
	WORD ver = MAKEWORD(2, 2);

	int wsInit = WSAStartup(ver, &data);

	return wsInit == 0;
}

// Receive loop
void CtcpServer::Run()
{
	char buf[MAX_BUFFER_SIZE];
	SOCKET listening = createSocket();
	if (listening == INVALID_SOCKET)
	{
		return;
	}
	FD_SET(listening, &_clientList);

	while (true)
	{
		fd_set tempClientList = _clientList;

		int socketCount = select(0, &tempClientList, nullptr, nullptr, nullptr);

		for (int i = 0;i < socketCount;i++)
		{
			SOCKET sock = tempClientList.fd_array[i];
			if (sock == listening)
			{
				SOCKET newClient = accept(listening, nullptr, nullptr);

				FD_SET(newClient, &_clientList);

				std::string welcomeMessage = "Welcome to the server!\n";
				send(newClient, welcomeMessage.c_str(), welcomeMessage.size() + 1, 0);

				if (MessageReceived != nullptr)
				{
					MessageReceived(this, newClient, listening, _clientList, "A client joined!\n");
				}
			}
			else
			{
				char buf[MAX_BUFFER_SIZE];
				memset(buf, 0, MAX_BUFFER_SIZE);

				int bytesReceived = recv(sock, buf, MAX_BUFFER_SIZE, 0);
				if (bytesReceived <= 0)
				{
					closesocket(sock);
					FD_CLR(sock, &_clientList);
				}
				else
				{
					if (MessageReceived != nullptr)
					{
						MessageReceived(this, sock, listening, _clientList, std::string(buf, 0, bytesReceived));
					}
				}
			}
		}
	}
}

// Clean up
void CtcpServer::cleanUp()
{
	FD_ZERO(&_clientList);
	WSACleanup();
}

// Create a socket
SOCKET CtcpServer::createSocket()
{
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening != 0)
	{
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(_port);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;

		int bindOk = bind(listening, (sockaddr*)&hint, sizeof(hint));
		if (bindOk != SOCKET_ERROR)
		{
			int listenOk = listen(listening, SOMAXCONN);
			if (listenOk == SOCKET_ERROR)
			{
				return -2;
			}
		}
		else
		{
			return -1;
		}
	}
	return listening;
}
