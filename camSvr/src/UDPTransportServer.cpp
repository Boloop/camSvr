/*
 * UDPTransportServer.cpp
 *
 *  Created on: 5 Apr 2011
 *      Author: two
 */

#include "UDPTransportServer.h"

bool byteMatch(char* a, char* b, int len)
{
	/*
	 * Will return if the binaries match, strcmp not safe, may not be nulled!
	 */

	int i;
	for(i = 0; i<len; i++)
	{
		if(a[i] != b[i]) return false;
	}

	return true;

}


UDPTransportServer::UDPTransportServer()
{
	// TODO Auto-generated constructor stub


	m_vConnectionList = new vector<UDPTransportStreamer*>();
	m_vConnectionRemovalList = new vector<int>();
	m_nMaxConnections = 10;
	m_eTransportID = TRANSPORT_UDP;
	m_mConListLock = PTHREAD_MUTEX_INITIALIZER;

	//alloc Buffer
	m_nBufferSize = 1024;
	m_pBuffer = (char*)malloc(m_nBufferSize);

	m_slFromLen = sizeof(m_CliAddr);
	m_nPortNo = 5125;

	m_bBroadcaster = false;






}

void UDPTransportServer::Execute()
{
	if(m_bBroadcaster)
		return; //Broadcasting doesn't listen at all! :O

	vector<UDPTransportStreamer*>::iterator iList;

	//struct sockaddr_in cli_addr;
	m_ServAddr.sin_family = AF_INET;
	m_ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_ServAddr.sin_port = htons(m_nPortNo);
	m_fdSocket = socket(AF_INET, SOCK_DGRAM, 0);

	int bindResult = bind(m_fdSocket, (struct sockaddr *) &m_ServAddr, sizeof(m_ServAddr) );
	cout << "UDP Bind Result " << bindResult << endl;

	int rLength;

	while(true){
		rLength = recvfrom(m_fdSocket, m_pBuffer, m_nBufferSize, 0, (struct sockaddr *)&m_CliAddr, &m_slFromLen);

		if(rLength == 4)
		{
			//Check out what it says.
			cout << "4 is a magic number" << endl;
			if (byteMatch("STRT" ,m_pBuffer, 4))
				cout << "start cmd" << endl;

			if (byteMatch("STOP" ,m_pBuffer, 4))
				cout << "stop cmd" << endl;

			if (byteMatch("INFO" ,m_pBuffer, 4))
				cout << "information cmd" << endl;


		}
		else
		{
			cout << "UDP wrong packet size, IGNORE" << endl;
			continue;
		}

		pthread_mutex_lock(&m_mConListLock);

		//Is this connection here?
		for(iList = m_vConnectionList->begin(); iList != m_vConnectionList->end(); ++iList)
		{
			if( (*iList)->isAddr(&m_CliAddr) )
			{
				cout << "It's there! It's THERE" << endl;
			}

			struct timeval now;
			gettimeofday(&now, NULL);
			(*iList)->gotSTRTat(&now);


		}// for each connection...


		//Nope, create a streamer!
		if(m_vConnectionList->size() < m_nMaxConnections) //Do we have some space?
		{
			//Create, add to list and launch!
			UDPTransportStreamer *ss = new UDPTransportStreamer(m_CliAddr, m_fdSocket, m_slFromLen);
			m_vConnectionList->push_back(ss);
			cout << "ACCEPTED" << endl;

			ss->setTimeOut(5);

			ss->Start();
		}


		pthread_mutex_unlock(&m_mConListLock);


	}// while true!

	return;



}

void UDPTransportServer::sendData(char* buffer, int size)
{
	pthread_mutex_lock(&m_mConListLock);
	vector<UDPTransportStreamer*>::iterator iList;


	if (m_bBroadcaster) //OK test
	{
		sendto(m_fdSocket, buffer, size, 0, (struct sockaddr *)&m_CliAddr, sizeof(m_CliAddr));
		return;
	}

	int i=0;

	for(iList = m_vConnectionList->begin(); iList != m_vConnectionList->end(); ++iList)
	{
		//(*iList)->sendPacket(buffer, size);
		int erron;

			erron = sendto(m_fdSocket, buffer, size, 0, (struct sockaddr *)&((*iList)->m_CliAddr), m_slFromLen);
		if(erron < 0)
			cout << "Whoops :S" << endl; // This should never happen?
		//cout << "Is it dead?" << endl;

		//is it dead?
		struct timeval now;
		gettimeofday(&now, NULL);
		(*iList)->sendPacket(NULL, 0); //Just to check if dead

		if( (*iList)->isClientDead() )
		{ //Client is dead, kill it and remove it from the streets
			m_vConnectionRemovalList->push_back(i);
		}

		//cout << erron << endl;

		i++;
	}// for each connection


	//Remove dead connections
	while (m_vConnectionRemovalList->size() > 0)
	{
		cout << "Killing a connection :O" << endl;
		m_vConnectionList->erase( m_vConnectionList->begin() + m_vConnectionRemovalList->back() );
		m_vConnectionRemovalList->pop_back();
	}


	pthread_mutex_unlock(&m_mConListLock);
	return;
}

void UDPTransportServer::setBroadcast(bool value)
{
	m_bBroadcaster = value;

	if(m_bBroadcaster)
	{
		memset(&m_CliAddr, 0x00, sizeof(struct sockaddr_in));
		m_CliAddr.sin_family = AF_INET;
		m_CliAddr.sin_port = (in_port_t)htons(m_nPortNo);
		//m_CliAddr.sin_addr =
		m_CliAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

		m_fdSocket = socket(AF_INET, SOCK_DGRAM, 0);
		m_slFromLen = sizeof(m_CliAddr);


	}

}

void UDPTransportServer::setPort(int value)
{
	m_nPortNo = value;
}

void UDPTransportServer::Setup()
{
	return;
}

UDPTransportServer::~UDPTransportServer()
{
	// TODO Auto-generated destructor stub
}
