#pragma once

#include<iostream>
#include<WS2tcpip.h>
#include<string>

#pragma comment (lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE (32)

class CtcpServer;

typedef void(*MessageReceivedHandler)(CtcpServer* server, int senderSocket, int listeningSocket, fd_set &clientList, std::string msg);

class CtcpServer
{
	int						_port;
	fd_set					_clientList;
	MessageReceivedHandler	MessageReceived;
	
	SOCKET createSocket();

	SOCKET Accept(int listening);

	void disconnectClient(int sock);

public:

	CtcpServer(int port, MessageReceivedHandler handler);

	~CtcpServer();

	void Send(int senderSocket, int listeningSocket, fd_set &clientList, std::string msg);

	// Initialize winsock
	bool Init();

	// Receive loop
	void Run();

	// Clean up
	void cleanUp();
};