/*
 * NetCommander.cpp
 *
 *  Created on: 2 Mar 2011
 *      Author: two
 */

#include "NetCommander.h"



NetCommander::NetCommander(int port) {
	// TODO Auto-generated constructor stub
	m_nPort = port;
}

int NetCommander::bindSocket(){
	/*
	 * Will bind to port and return and errors! muha ha ha ha ha.
	 */

	m_ServAddr.sin_family = AF_INET;
	m_ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_ServAddr.sin_port = htons(m_nPort);
	m_fdSocket = socket(AF_INET, SOCK_STREAM, 0);

	return bind( m_fdSocket, (struct sockaddr *) &m_ServAddr, sizeof(m_ServAddr) );

}

void NetCommander::Setup()
{
	//Setup
}

void NetCommander::Execute()
{
	/*
	 * This will listen and conenct to the first connection.
	 */

	struct sockaddr_in cliAddr;
	int clientSocket;
	char* buffer;
	int buffersize = 1024;
	int msgSize;
	buffer = (char*)malloc(buffersize);


	if ( listen(m_fdSocket, 1) < 0 ) {
		//printf("ECHOSERV: Error calling listen()\n");
		return;
	}

	if ( (clientSocket = accept(m_fdSocket, NULL, NULL) ) < 0 ) {
		   //printf("ECHOSERV: Error calling accept()\n");
		    return;
	}

	//std::cout << "Connection From: " << nltoh() << std::endl;

	while(1){

		//
		msgSize = recv(clientSocket, buffer, buffersize, 0);

		//Process command
		std::cout << "msg Size: " << msgSize << std::endl;
		std::cout << buffer << std::endl;


		send(clientSocket, buffer, msgSize, 0);
		//Readline(clientSocket, buffer, buffersize);
		//Writeline(clientSocket, buffer, strlen(buffer));
	}

}

NetCommander::~NetCommander() {
	// TODO Auto-generated destructor stub
}
