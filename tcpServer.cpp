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
		if (sock != senderSocket && sock != listeningSocket)
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
				SOCKET newClient = Accept(listening);

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
					disconnectClient(sock);
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

void CtcpServer::disconnectClient(int sock)
{
	std::cout << "Someone DCed!\n";
	closesocket(sock);
	FD_CLR(sock, &_clientList);
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

SOCKET CtcpServer::Accept(int listening)
{
	sockaddr_in clientInfo;
	int clientSize = sizeof(clientInfo);

	SOCKET newClient = accept(listening, (sockaddr*)&clientInfo, &clientSize);

	char host[NI_MAXHOST];		// Client's remote name
	char service[NI_MAXSERV];	// Service (port) the client is connected on

	memset(host, 0, NI_MAXHOST);
	memset(service, 0, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&clientInfo, sizeof(clientInfo), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << " connected on port " << service << std::endl;
	}
	else
	{
		inet_ntop(AF_INET, &clientInfo.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connceted on port " << ntohs(clientInfo.sin_port) << std::endl;
	}
	return newClient;
}