/*
 * Packetiser.h
 *
 *  Created on: 6 Apr 2011
 *      Author: two
 *
 *  this will do all the Muxing/Container/PAcketising of the stream(s)
 *  All will hold the gory details of the lot, muha ha ha ha ha ha ha
 *  well hopefully it's not that gory :S
 *
 *  This is designed to work with my IOBuffer class.
 */





#ifndef PACKETISER_H_
#define PACKETISER_H_

//Bug? with INT64_C not being decalred in scope. Silly C++
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif //ifndef INT64_C

#include <iostream>
#include "MP4HEADER.h"
#include "VideoCompressor.h"
extern "C" {
#include <sys/time.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avio.h>

}
#include "IOBuffer.h"



struct timedif
{
	__time_t dsec;
	__suseconds_t dusec;
};

void timeDifCalc(struct timeval* start, struct timeval* stop, struct timedif* td);



class Packetiser
{
private:
	AVOutputFormat     *m_OutputFormat;
	AVFormatContext    *m_FormatCtx;
	char*				m_sPacketiserName;
	struct timeval		m_tvStartRefference; // When t=0 started reference the Computer Clock!
	bool				m_bFoundFormat;
	AVStream		   *m_VideoStream;
	AVRational			m_timeBase;
	int64_t				m_nLastPTS;
	ByteIOContext      *m_ByteIOCtx;
	ByteIOContext      *m_ByteIOCtxURL;

	bool				m_bMP4HeaderHack;

public:
	Packetiser(char* type);
	int addVideoStream(AVCodecContext * codecCtx);
	void setTimeBase(int num, int den);
	void setByteIOContext(ByteIOContext* ioctx);
	int initPacketiser();
	int writeInPacket(AVPacket* pac);
	int setPacketStamps(AVPacket* pac, struct timeval* timeOfPacket);
	int addTrailer();
	void setMP4HeaderHack(bool val);
	const char* getName();
	virtual ~Packetiser();

};




/*
 *  this is a moov box for mp4v only track codec.
 *  and loads of other issues :/
 *
 */





#endif /* PACKETISER_H_ */
