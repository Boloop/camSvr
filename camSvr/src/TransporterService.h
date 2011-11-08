/*
 * TransporterService.h
 *
 *  Created on: 11 Mar 2011
 *      Author: two
 *
 *      This is used as a template for all "Transporting" classes such as HTTP, RTP etc
 *      Not threaded. But you can inherit from two classes to make it so :)
 */

#ifndef TRANSPORTERSERVICE_H_
#define TRANSPORTERSERVICE_H_
#include <string>


enum TransportID {
	TRANSPORT_NONE,
	TRANSPORT_HTTP,
	TRANSPORT_RTP,
	TRANSPORT_UDP,
};



class TransporterService {

protected:
	int 					m_nPortListen;
	enum TransportID		m_eTransportID;

public:
	TransporterService();
	enum TransportID getTransportID();
	int getListeningPort();

	virtual void sendData(char *buffer, int lenght);

	virtual ~TransporterService();
};

#endif /* TRANSPORTERSERVICE_H_ */
