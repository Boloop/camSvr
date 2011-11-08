/*
 * HTTPServer.cpp
 *
 *  Created on: 11 Mar 2011
 *      Author: two
 */

#include "HTTPServer.h"

HTTPServer::HTTPServer() {
	// TODO Auto-generated constructor stub
	m_nPortListen = 8080;
	//vector<HTTPStreamer*>  *m_vConnectionList
	//m_vConnectionList = new vector<HTTPStreamer*>;
	m_nMaxConnections = 10;
	m_eTransportID = TRANSPORT_HTTP;

	//Create the list
	m_vConnectionList = new vector<HTTPStreamer*>();
	m_vConnectionRemovalList = new vector<int>();
	m_mConListLock = PTHREAD_MUTEX_INITIALIZER;

	m_bFirstEnabled = false;
	m_pFirstListLength = NULL;
	m_pFirstList = NULL;
	m_nFirstListSize = 0;

	m_bPacketiserType = NULL;
	m_bPacketiserCodecCtx = NULL;

}

void HTTPServer::Setup()
{
	/*
	 * Will do nothing :(
	 */

	return;
}

void HTTPServer::setFirstList(uint8_t ** buffs, int* lengths, int size)
{
	/*
	 * This will be the very first packets that the server will send (after header)
	 * for each connection! Will be passed to HTTPStreamer classes...
	 */
	m_bFirstEnabled = true;
	m_pFirstListLength = lengths;
	m_pFirstList = buffs;
	m_nFirstListSize = size;

}

void HTTPServer::setListenPort(int port)
{
	m_nPortListen = port;
}

void HTTPServer::sendData(char *buffer, int length)
{
	/*
	 * Will send data to all active sockets :)
	 */
	int i=0;
	pthread_mutex_lock(&m_mConListLock);

	vector<HTTPStreamer*>::iterator iList;

	for(iList = m_vConnectionList->begin(); iList != m_vConnectionList->end(); ++iList) // for each connection
	{
		(*iList)->sendPacket(buffer, length);
		if( (*iList)->isClientDead() )
		{ //Client is dead, kill it and remove it from the streets
			m_vConnectionRemovalList->push_back(i);

		}


		++i;
	}

	//Remove dead connections is list
	while (m_vConnectionRemovalList->size() > 0)
	{
		m_vConnectionList->erase( m_vConnectionList->begin() + m_vConnectionRemovalList->back() );
		m_vConnectionRemovalList->pop_back();
	}

	pthread_mutex_unlock(&m_mConListLock);
	return;
}

void HTTPServer::packitiseAndSend(AVPacket *packet, bool isIframe)
{
	/*
	 * Each connection will packetise and then send across :)
	 */
	int i=0;
		pthread_mutex_lock(&m_mConListLock);

		vector<HTTPStreamer*>::iterator iList;

		for(iList = m_vConnectionList->begin(); iList != m_vConnectionList->end(); ++iList) // for each connection
		{
			(*iList)->packitiseAndSend(packet, isIframe);
			if( (*iList)->isClientDead() )
			{ //Client is dead, kill it and remove it from the streets
				m_vConnectionRemovalList->push_back(i);

			}


			++i;
		}

		//Remove dead connections is list
		while (m_vConnectionRemovalList->size() > 0)
		{
			m_vConnectionList->erase( m_vConnectionList->begin() + m_vConnectionRemovalList->back() );
			m_vConnectionRemovalList->pop_back();
		}

		pthread_mutex_unlock(&m_mConListLock);
		return;
}

void HTTPServer::setUpPacketiser(char* type, AVCodecContext * codecCtx)
{
	m_bPacketiserType = type;
	m_bPacketiserCodecCtx = codecCtx;
}

void HTTPServer::Execute()
{
	/*
	 * Will run the listen loop and keep spinning around, right round
	 * like a record baby.
	 */

	int clientSocket;
	int result;

	//Setup the Listening Socket and bind.
	m_ServAddr.sin_family = AF_INET;
	m_ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_ServAddr.sin_port = htons(m_nPortListen);
	m_fdSocket = socket(AF_INET, SOCK_STREAM, 0);

	result = bind( m_fdSocket, (struct sockaddr *) &m_ServAddr, sizeof(m_ServAddr) );
	if(result != 0)
	{
		cout << "Could not bind :(" << endl;
		return;
	}
	else
	{
		cout << "Binded! Listening on: " << m_nPortListen << endl;
	}

	//listen for connections
	if ( listen(m_fdSocket, 3) < 0 ) {
					//fprintf("ECHOSERV: Error calling listen()\n");
					return;
				}
	//Accept! :)
	while(true){

		if ( (clientSocket = accept(m_fdSocket, NULL, NULL) ) < 0 ) {
					   //printf("ECHOSERV: Error calling accept()\n");
					    return;
				}
		cout << "Got a connection " << m_vConnectionList->size()+1 << "out of " << m_nMaxConnections <<endl;
		//pthread_mutex_lock
		pthread_mutex_lock(&m_mConListLock);//Lock to Acess List

		if(m_vConnectionList->size() < m_nMaxConnections) //Do we have some space?
		{
			//Create, add to list and launch!
			HTTPStreamer *ss = new HTTPStreamer(clientSocket);
			m_vConnectionList->push_back(ss);
			cout << "ACCEPTED" << endl;

			if(m_bFirstEnabled)
			{
				//m_pFirstListLength = lengths;
				//m_pFirstList = buffs;
				//m_nFirstListSize = size;
				ss->setFirstList(m_pFirstList, m_pFirstListLength, m_nFirstListSize);
			}

			if(m_bPacketiserCodecCtx != NULL) // Setup Packetiser!
			{
				if( ss->setUpPacketiser(m_bPacketiserType, m_bPacketiserCodecCtx) != 0)
				{
					cout << "PACKETISER FAILED TO INIT :(" << endl;
				}
			}

			ss->Start();
		}
		else
		{
			//Too many connections :(
			HTTPStreamer *ss = new HTTPStreamer(clientSocket);
			ss->rejectConnection();
			close(clientSocket);
			delete ss;
			cout << "REJECTED" << endl;

		}
		//pthread_mutex_unlock
		pthread_mutex_unlock(&m_mConListLock);



	} // While(true)


}

HTTPServer::~HTTPServer() {
	// TODO Auto-generated destructor stub
}

