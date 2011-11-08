/*
 * UDPTransportServer.h
 *
 *  Created on: 5 Apr 2011
 *      Author: two
 */

#ifndef UDPTRANSPORTSERVER_H_
#define UDPTRANSPORTSERVER_H_

#include "TransporterService.h"
#include "MyThread.h"
#include "UDPTransportStreamer.h"
#include <stdlib.h>
#include <iostream>
#include <string.h> //for memset
#include <vector>
#include <time.h>


extern "C" {
	#include <sys/socket.h>
	#include <sys/types.h>        /*  socket types              */
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <pthread.h>
};

using namespace std;

class UDPTransportServer: public MyThread, public TransporterService
{
private:
	struct sockaddr_in 							 m_ServAddr;
	struct sockaddr_in							 m_CliAddr;
	int 										 m_fdSocket;
	unsigned int								 m_nMaxConnections;
	int											 m_nBufferSize;
	char 										*m_pBuffer;
	socklen_t									 m_slFromLen;
	int											 m_nPortNo;

	bool										 m_bBroadcaster;

	vector<UDPTransportStreamer*>  				*m_vConnectionList;
	vector<int>  								*m_vConnectionRemovalList; //yeah, a bloody monster
	pthread_mutex_t 							 m_mConListLock;

public:
	UDPTransportServer();
	virtual ~UDPTransportServer();

	void setBroadcast(bool value);
	void setPort(int value);

	//Methods to override
	void sendData(char *buffer, int lenght);
	void Setup();
	void Execute();
};

#endif /* UDPTRANSPORTSERVER_H_ */
