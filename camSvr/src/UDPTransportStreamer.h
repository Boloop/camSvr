/*
 * UDPTransportStreamer.h
 *
 *  Created on: 5 Apr 2011
 *      Author: two
 */

#ifndef UDPTRANSPORTSTREAMER_H_
#define UDPTRANSPORTSTREAMER_H_

#include "MyThread.h"
#include <iostream>
#include <time.h>
#include "Packetiser.h" // for timeDifCalc function

extern "C" {
	#include <sys/socket.h>
	#include <sys/types.h>        /*  socket types              */
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <pthread.h>
};

using namespace std;

class UDPTransportStreamer: public MyThread
{
public:
	struct sockaddr_in 				m_CliAddr;
	int								m_fdSocket;
	socklen_t						m_slFromLen;

	struct timeval					m_tvLastSTRT;
	int								m_nTimeOut; // time out of STRT to send messages.
	bool							m_bClientDead;

public:
	UDPTransportStreamer();
	UDPTransportStreamer(struct sockaddr_in addr, int fdSocket, socklen_t fromLen);

	bool isAddr(struct sockaddr_in *addr);
	void setTimeOut(int sec);
	void gotSTRTat(struct timeval* time);
	bool isClientDead();

	void Setup();
	void Execute();

	void sendPacket(char* buffer, int size);

	virtual ~UDPTransportStreamer();
};

#endif /* UDPTRANSPORTSTREAMER_H_ */
