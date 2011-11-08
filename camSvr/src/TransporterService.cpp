/*
 * TransporterService.cpp
 *
 *  Created on: 11 Mar 2011
 *      Author: two
 */

#include "TransporterService.h"

TransporterService::TransporterService() {
	// TODO Auto-generated constructor stub

	m_eTransportID = TRANSPORT_NONE;
	m_nPortListen = 0;

}

int TransporterService::getListeningPort()
{
	/*
	 * Return the port the service is listening on
	 */
	return m_nPortListen;
}

enum TransportID TransporterService::getTransportID()
{
	/*
	 * Return the TransportID of the Transporter Service
	 */
	return m_eTransportID;
}

void TransporterService::sendData(char* buffer, int length)
{
	/*
	 * This will be fed in a whole packet of data to be sent to any connected machines.
	 * This will be overided by all the Transporter Services.
	 */
	return;
}

TransporterService::~TransporterService() {
	// TODO Auto-generated destructor stub
}
