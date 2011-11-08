/*
 * VideoCompressor.h
 *
 *  Created on: 28 Feb 2011
 *      Author: two
 *
 *  This is used to Encode the frames, currently Setup to use H.262 (MPEG2-video) but no reason as to
 *  why it can be adapted to others.
 *
 */

#ifndef VIDEOCOMPRESSOR_H_
#define VIDEOCOMPRESSOR_H_


//Bug? with INT64_C not being decalred in scope. Silly C++
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif //ifndef INT64_C

extern "C" {
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavcodec/avcodec.h>
	#include <pthread.h>
}



class VideoCompressor {

private:
	AVCodec *			m_ptrCodec;
	AVCodecContext *	m_ptrCodecCtx;
	int 				m_nEncodingWidth;
	int 				m_nEncodingHeight;
	int					m_nEncodingBitRate;
	enum PixelFormat	m_eColourSpace;
	int					m_nGOPSize;
	enum CodecID		m_eCodecID;
	bool				m_bUseGlobalHeaders; // True for MP4, Default False. Stops error for mp4 Packetising


public:
	VideoCompressor();
	int setupEncoder();

	void setWidthHeight(int width, int height);
	void setBitRate(int rate);
	void setColourSpace(enum PixelFormat pixformat);
	void setGOPSize(int size);
	void setGlobalHeaders(bool value);
	void setCodecID(enum CodecID codecid);

	int encodeFrame(AVFrame * frame, char * buffer, int bufferLength, int * writtenSize);

	AVCodecContext* getCodecContext();

	virtual ~VideoCompressor();
};

#endif /* VIDEOCOMPRESSOR_H_ */
