/*
 * CamCapture.h
 *
 *  Created on: 6 Feb 2011
 *      Author: one
 *
 *  Using the v4l2 API to interface with the camera, supports MMAP mode only!
 *  This has built in feature of using swscale to scale the frame down (or up)
 *  as well as change colourspace.
 */

#ifndef CAMCAPTURE_H_
#define CAMCAPTURE_H_

#include <stdlib.h>
#include <sys/stat.h> //for Stat contex
#include <sys/fcntl.h> //for opening the device
//#include <stdlib.h> //exit

#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#include <sys/mman.h> // for munmap()

#include <string.h> //has memset

//Bug with INT64_C not being decalred in scope. Silly C++
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C" {
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavcodec/avcodec.h>
}

//#include <stdint.h>

struct buffer {
        void *                  start;
        size_t                  length;
};

static int
xioctl                          (int                    fd,
                                 int                    request,
                                 void *                 arg);

struct CamCap_buffer {
        void *                  start;
        size_t                  length;
};

class CamCapture {

private:
	char* 					m_sDevicePath;
	bool 					m_bDeviceOpen;
	bool 					m_bDeviceInitd;
	bool					m_bDeviceCapturing;
	bool					m_bDeviceInitdConversionBuffer;
	int 					m_fileDescriptor;
	int						m_nBufferCount;
	struct buffer *         m_ptrBuffers;
	unsigned int			m_uBufferCount;

	AVFrame	*				m_ptrCamFrame;
	struct SwsContext *		m_pSwsContext;
	int						m_nScalingHeight;



public:
	CamCapture();
	CamCapture(char* sDev);

	int openDevice();
	//int initDevice();
	unsigned long int capabilities();
	int closeDevice();
	bool isOpen();

	int initDevice();
	int uninitDevice();
	bool isInitd();

	int startCapture();
	int stopCapture();
	bool isCapturing();

	int initConversionBuffer();
	int uninitConversionBuffer();
	bool isInitdConversionBuffer();

	int setConverstion(int width, int height, enum PixelFormat pixFormat);
	AVFrame* allocFrame(int width, int height, enum PixelFormat pixFormat);

	int grabPicture(AVFrame* picture);

	virtual ~CamCapture();


};

#endif /* CAMCAPTURE_H_ */
