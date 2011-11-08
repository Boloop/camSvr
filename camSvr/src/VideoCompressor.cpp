/*
 * VideoCompressor.cpp
 *
 *  Created on: 28 Feb 2011
 *      Author: two
 *
 */

#include "VideoCompressor.h"

VideoCompressor::VideoCompressor() {
	// TODO Auto-generated constructor stub
	m_ptrCodecCtx = NULL;
	m_ptrCodec = NULL;

	m_nEncodingWidth = 160;
	m_nEncodingHeight = 120;
	m_nEncodingBitRate = 1280000;
	//m_eColourSpace = PIX_FMT_YUYV422;

	m_eColourSpace = PIX_FMT_YUV420P;



	m_nGOPSize = 10;

	m_bUseGlobalHeaders = false;

	m_eCodecID = CODEC_ID_MPEG2VIDEO;
}

int VideoCompressor::setupEncoder()
{
	/*
	 * this will find the codec and set up the parameters for the codecCtx as specified
	 * by the set methods.
	 */
	avcodec_register_all();

	if (m_ptrCodec != NULL)
		free(m_ptrCodec);

	m_ptrCodec = avcodec_find_encoder(m_eCodecID);

	if(m_ptrCodec == NULL)
	{
		fprintf(stderr, "could not find codec\n");
		return -1;
	}

	if (m_ptrCodecCtx == NULL)
		m_ptrCodecCtx = avcodec_alloc_context();

	m_ptrCodecCtx->width = m_nEncodingWidth;
	m_ptrCodecCtx->height = m_nEncodingHeight;
	m_ptrCodecCtx->pix_fmt = m_eColourSpace;
	m_ptrCodecCtx->gop_size = m_nGOPSize;
	/* Copy Pasta */
	m_ptrCodecCtx->max_b_frames=0; //h263 doesnt have any b-frames
	m_ptrCodecCtx->codec_id = m_eCodecID;
	m_ptrCodecCtx->codec_type = CODEC_TYPE_VIDEO;
	m_ptrCodecCtx->time_base.num = 1;
	m_ptrCodecCtx->time_base.den = 30;
	//m_ptrCodecCtx->pix_fmt = PIX_FMT_YUV420P;
	if (m_bUseGlobalHeaders)
		m_ptrCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER; // Place global headers in extradata instead of every keyframe.
	m_ptrCodecCtx->bit_rate = m_nEncodingBitRate;

	if (avcodec_open(m_ptrCodecCtx, m_ptrCodec) < 0) {
		fprintf(stderr, "could not open codec\n");
		return -2;
	}

	return 0;
}

int VideoCompressor::encodeFrame(AVFrame * frame, char * buffer, int bufferLength, int* writtenSize)
{
	/*
	 * this will get a frame That is already the right Width and Height and Pixformat for this codecCtx
	 * and write the stream to the buffer supplied.
	 */

	*writtenSize = avcodec_encode_video(m_ptrCodecCtx, (uint8_t*)buffer, bufferLength, frame );
	return 0;

}

void VideoCompressor::setGlobalHeaders(bool value)
{
	/*
	 * Place global headers in extradata instead of every keyframe.
	 * Mp4 codec does not place by default this value into keyframe, however It
	 * needs to be setto false to get mpegts to work... :(
	 * but set to true for mp4 container/packetiser work. :/
	 */
	m_bUseGlobalHeaders = value;
}

void VideoCompressor::setWidthHeight(int width, int height)
{
	/*
	 * Set the Width and Height of the Frames to be encoded
	 * MUST BE CALLED BEFORE SETTING UP THE ENCODER!
	 */
	m_nEncodingWidth = width;
	m_nEncodingHeight = height;
}

void VideoCompressor::setBitRate(int rate)
{
	/*
	 * Set the bitrate of the encoded stream. (Unsure of units, probably varies from encoder, to encoder)
	 * MUST BE CALLED BEFORE SETTING UP THE ENCODER!
	 */
	m_nEncodingBitRate = rate;
}

void VideoCompressor::setColourSpace(enum PixelFormat pixformat)
{
	/*
	 * This will set the Pixel Format/colourspace being given in.
	 * MUST BE CALLED BEFORE SETTING UP THE ENCODER!
	 */
	m_eColourSpace = pixformat;
}
void VideoCompressor::setGOPSize(int size)
{
	/*
	 * This will set the GOP size (I + number of P frames afterwards)
	 * MUST BE CALLED BEFORE SETTING UP THE ENCODER!
	 */
	m_nGOPSize = size;
}

void VideoCompressor::setCodecID(enum CodecID codecid)
{
	/*
	 * this will setup the coder ID to be used.
	 * MUST BE CALLED BEFORE SETTING UP THE ENCODER!
	 */
	m_eCodecID = codecid;
}

AVCodecContext * VideoCompressor::getCodecContext()
{
	return m_ptrCodecCtx;
}

VideoCompressor::~VideoCompressor() {
	// TODO Auto-generated destructor stub
}
