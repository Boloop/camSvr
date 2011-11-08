/*
 * HTTPStreamer.h
 *
 *  Created on: 2 Mar 2011
 *      Author: two
 *
 *  Currently used to transport data as a application/octetstream in HTTP. Just ignores the header/request being
 *  received and gives a HTTP/1.0 200 OK status.  This class is paritally threaded. But does not listen, pass in a
 *  socket of a premade connection.
 */

#ifndef HTTPSTREAMER_H_
#define HTTPSTREAMER_H_

#include <iostream>
using namespace std;

extern "C" {
	#include <stdlib.h>
	#include <sys/socket.h>
	#include <sys/types.h>        /*  socket types              */
	#include <netinet/in.h>
	#include <arpa/inet.h>
//#include <sys/socket.h>       /*  socket definitions        */
//#include <sys/types.h>        /*  socket types              */
//#include <arpa/inet.h>        /*  inet (3) funtions         */
	#include <unistd.h>           /*  misc. UNIX functions      */
	#include <pthread.h>


}


#include "MyThread.h"
#include "Packetiser.h"
#include "IOBuffer.h"

class HTTPStreamer: public MyThread {

private:
	int						m_fdSocket;
	struct sockaddr_in 		m_CliAddr;
	bool 					m_bHeaderSent;
	pthread_mutex_t 		m_mHeaderSentLock; // Lock used for both HeaderSent AND ClientDead.
	bool					m_bClientDead;


	int 										*m_pFirstListLength;
	uint8_t 								   **m_pFirstList;
	int											 m_nFirstListSize;
	bool										 m_bFirstEnabled;

	Packetiser 									*m_Packetiser; // this is an experiment.
	IOBuffer									*m_IOBuffer;
	bool										 m_bSentFirstIFrame;
	bool										 m_bMP4HeaderHack; //The hack that injects the moov box of mp4v stream.



public:
	HTTPStreamer(int socket);
	int setUpPacketiser(char* type, AVCodecContext * codecCtx);

	void setFirstList(uint8_t ** buffs, int* lengths, int size);
	void Setup();
	void Execute();
	void rejectConnection();
	void sendPacket(char* buffer, int size);
	bool isClientDead();

	void packitiseAndSend(AVPacket *packet, bool isIframe);

	virtual ~HTTPStreamer();
};

#endif /* HTTPSTREAMER_H_ */
