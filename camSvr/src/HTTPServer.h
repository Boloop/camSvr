/*
 * HTTPServer.h
 *
 *  Created on: 11 Mar 2011
 *      Author: two
 *
 *      This Class will deal with HTTP requests
 */

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_
#include "TransporterService.h"
#include "MyThread.h"
#include "HTTPStreamer.h"
#include <vector>
#include <iostream>
//#include <boost/regex.hpp> //Not used and BB doesn't have the library (too lazy to install)
using namespace std;

extern "C" {
	#include <sys/socket.h>
	#include <sys/types.h>        /*  socket types              */
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <pthread.h>
};

using namespace std;


class HTTPServer : /*public TransporterService ,*/ public MyThread, public TransporterService{

private:
	struct sockaddr_in 							 m_ServAddr;
	int 										 m_fdSocket;
	int 										 m_nMaxConnections;
	//int									       m_nPortListen;
	vector<HTTPStreamer*>  						*m_vConnectionList;
	vector<int>  								*m_vConnectionRemovalList; //yeah, a bloody monster
	pthread_mutex_t 							 m_mConListLock;

	int 										*m_pFirstListLength;
	uint8_t 								   **m_pFirstList;
	int											 m_nFirstListSize;
	bool										 m_bFirstEnabled;

	char										*m_bPacketiserType;
	AVCodecContext								*m_bPacketiserCodecCtx;





public:

	HTTPServer();
	virtual ~HTTPServer();

	void setMaxConnections(int number);
	void setFirstList(uint8_t ** buffs, int* lengths, int size);
	void setListenPort(int port);

	void setUpPacketiser(char* type, AVCodecContext * codecCtx);
	void packitiseAndSend(AVPacket *packet, bool isIframe);

	//Methods to override
	void sendData(char *buffer, int lenght);
	void Setup();
	void Execute();





};

#endif /* HTTPSERVER_H_ */
