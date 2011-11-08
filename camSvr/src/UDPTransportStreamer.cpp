/*
 * UDPTransportStreamer.cpp
 *
 *  Created on: 5 Apr 2011
 *      Author: two
 */

#include "UDPTransportStreamer.h"

UDPTransportStreamer::UDPTransportStreamer()
{
	// TODO Auto-generated constructor stub

}

UDPTransportStreamer::UDPTransportStreamer(struct sockaddr_in addr, int fdSocket, socklen_t fromLen)
{
	m_CliAddr = addr;
	m_fdSocket = fdSocket;
	m_slFromLen = fromLen;

	m_nTimeOut = 0; // 0 = none
	m_bClientDead = false;


}

void UDPTransportStreamer::setTimeOut(int sec)
{
	/*
	 * Set the time out in seconds. 0 = none.
	 * timeout for last STRT command. Will destroy self after that.
	 */

	m_nTimeOut = sec;
}

void UDPTransportStreamer::Setup()
{
	return;
}

void UDPTransportStreamer::Execute()
{
	return;
}

void UDPTransportStreamer::gotSTRTat(struct timeval* time)
{
	/*
	 * Marks this as the last time it got a STRT.
	 */

	m_tvLastSTRT.tv_sec = time->tv_sec;
	m_tvLastSTRT.tv_usec = time->tv_usec;

}


bool UDPTransportStreamer::isAddr(struct sockaddr_in *addr)
{
	/*
	 * Returns true if the port and addr is matched to that given in :)
	 */

	if(addr->sin_port == m_CliAddr.sin_port)
	{
		if(addr->sin_addr.s_addr == m_CliAddr.sin_addr.s_addr)
			return true;
	}

	return false;
}

bool UDPTransportStreamer::isClientDead()
{
	return m_bClientDead;
}

void UDPTransportStreamer::sendPacket(char* buffer, int size)
{

	if(m_bClientDead)
		return; //Don't send to dead clients...

	// Check last STRT?
	if(m_nTimeOut)
	{
		struct timedif td;
		struct timeval now;
		gettimeofday(&now, NULL);
		timeDifCalc(&m_tvLastSTRT, &now ,&td);
		if(td.dsec > m_nTimeOut)
		{
			//timed OUT!
			//m_bClientDead = true;
			return;
		}

		return; //Debug mode, code below is redundant...
	}


	int sendresult = sendto(m_slFromLen, buffer, size, 0, (struct sockaddr *)&m_CliAddr, m_slFromLen);
	cout << "Sending UDP data " << sendresult << endl;
	return;
}

UDPTransportStreamer::~UDPTransportStreamer()
{
	// TODO Auto-generated destructor stub
}
