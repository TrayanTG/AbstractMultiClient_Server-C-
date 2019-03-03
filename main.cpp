#include<iostream>
#include<string>

#include "tcpServer.h"

void Server_MessageReceived(CtcpServer* server, int senderSocket, int listeningSocket, fd_set &clientList, std::string msg);

int main()
{
	CtcpServer server(28199, Server_MessageReceived);

	if (server.Init())
	{
		server.Run();
	}

	return 0;
}

void Server_MessageReceived(CtcpServer* server, int senderSocket, int listeningSocket, fd_set &clientList, std::string msg)
{
	server->Send(senderSocket, listeningSocket, clientList, msg);
}