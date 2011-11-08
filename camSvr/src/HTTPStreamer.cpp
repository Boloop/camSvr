/*
 * HTTPStreamer.cpp
 *
 *  Created on: 2 Mar 2011
 *      Author: two
 */

#include "HTTPStreamer.h"

HTTPStreamer::HTTPStreamer(int socket)
{
	// TODO Auto-generated constructor stub

	m_fdSocket = socket;
	m_bHeaderSent = false;
	m_bClientDead = false;
	m_mHeaderSentLock = PTHREAD_MUTEX_INITIALIZER;
	//m_CliAddr = cliAddr;

	m_bFirstEnabled = false;
	m_pFirstListLength = NULL;
	m_pFirstList = NULL;
	m_nFirstListSize = 0;

	m_Packetiser = NULL;
	m_bSentFirstIFrame = false;
	m_bMP4HeaderHack = false;
}
void HTTPStreamer::setFirstList(uint8_t ** buffs, int* lengths, int size)
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


void HTTPStreamer::Setup()
{
	/*
	 * No setup needed, needed to be defined.
	 */
	return;
}

void HTTPStreamer::Execute(){
	/*
	 *  this will just send the response header for the HTTP protocol.
	 */
	int ans;
	//usleep(1000000);
	int rsize = 10000;
	char rbuf[10000];


	//Stops Sigpipe for broken pipe signals being called. Makes my
	// life easier when clients disconnect.
	//int set = 1;
	//setsockopt(m_fdSocket, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, sizeof(int));

	ans = recv(m_fdSocket, rbuf, rsize, 0);




	char buffer[] = "HTTP/1.0 200 OK\r\nContent-Type: application/octetstream\r\n\r\n";
	int size = 58;

	ans = send(m_fdSocket, buffer, size, MSG_NOSIGNAL);
	cout << "Header 200 Sent: " << ans << " vs " << size << endl;


	//Send firstdata
	if(m_bFirstEnabled)
	{
		int i;
		for (i = 0; i < m_nFirstListSize; i++)
		{
			ans = send(m_fdSocket, m_pFirstList[i], m_pFirstListLength[i], MSG_NOSIGNAL);
		}
	}

	pthread_mutex_lock(&m_mHeaderSentLock);
	m_bHeaderSent = true;
	if(ans < 0)
				m_bClientDead = true;
	pthread_mutex_unlock(&m_mHeaderSentLock);
}

void HTTPStreamer::rejectConnection()
{
	/*
	 * This will send a 500 to reject the client formally!
	 *  418 teapot is better tough >_>
	 */
	int ans;
	//usleep(1000000);

	pthread_mutex_lock(&m_mHeaderSentLock);
		if(!m_bHeaderSent && !m_bClientDead){

		char buffer[] = "HTTP/1.0 418 I'm a teapot\r\n\r\n";
		int size = 29;

		ans = send(m_fdSocket, buffer, size, 0);
		m_bHeaderSent = true;
		if(ans < 0)
			m_bClientDead = true;
	}
	pthread_mutex_unlock(&m_mHeaderSentLock);
}

void HTTPStreamer::sendPacket(char* buffer, int size)
{
	/*
	 * This method is used to send the actual binary data in the stream. Used after the header has been completed.
	 */

	int ans;
	pthread_mutex_lock(&m_mHeaderSentLock);



	if(m_bHeaderSent && !m_bClientDead)
	{
		ans = send(m_fdSocket, buffer, size, MSG_NOSIGNAL);
		//cout << "Ans: " << ans << "Len: " << size << endl;
		if(ans < 0)
					m_bClientDead = true;
	}
	pthread_mutex_unlock(&m_mHeaderSentLock);
}

void HTTPStreamer::packitiseAndSend(AVPacket *packet, bool isIframe)
{
	/*
	 * This will assume timestamping has been done? Maybe...
	 */

	if(!m_bSentFirstIFrame && isIframe)
		m_bSentFirstIFrame = true;

	if(!m_bSentFirstIFrame) // If not iFrame, and not sent one, go home :(
		return;

	m_Packetiser->writeInPacket( packet );

	ioBufferContext* hehe = (ioBufferContext*)m_IOBuffer->getByteIOContext()->opaque;
	int rofl = hehe->writeIndex - 1;
	if (rofl >= hehe->packetListSize || rofl < 0)
			rofl = 0;

	this->sendPacket((char*)hehe->packetList[rofl], hehe->packetLength[rofl]);
}

bool HTTPStreamer::isClientDead()
{
	/*
	 * Will return the value of m_bClientDead. Thread safe!
	 * clientDead will be flag true when there is any error sending data across.
	 */
	bool result;

	pthread_mutex_lock(&m_mHeaderSentLock);
	result = m_bClientDead;
	pthread_mutex_unlock(&m_mHeaderSentLock);

	return result;
}

int HTTPStreamer::setUpPacketiser(char* type, AVCodecContext * codecCtx)
{
	/*
	 * this will setup a Packetiser object for the stream.
	 * This is hoped that this will look like whatever is recieving it gets a whole
	 * file.
	 */

	/*
	Packetiser* mrPacket = new Packetiser( settings.sPacketiser );
		mrPacket->addVideoStream( encoder->getCodecContext() );
		mrPacket->setByteIOContext( ioBuffer->getByteIOContext() );
		if ( mrPacket->initPacketiser() != 0)
		{
			cout << "Failed to Init Packetiser. EXITING" << endl;
			exit(-1);
		}
		*/
	int result;
	pthread_mutex_lock(&m_mHeaderSentLock);
	m_Packetiser = new Packetiser( type );
	m_Packetiser->addVideoStream( codecCtx );

	m_IOBuffer = new IOBuffer();
	m_Packetiser->setByteIOContext( m_IOBuffer->getByteIOContext() );

	result = m_Packetiser->initPacketiser();
	pthread_mutex_unlock(&m_mHeaderSentLock);

	return result;



}

HTTPStreamer::~HTTPStreamer() {
	// TODO Auto-generated destructor stub
}
