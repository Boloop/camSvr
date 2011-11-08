/*
 * CamCapture.cpp
 *
 *  Created on: 6 Feb 2011
 *      Author: one
 */

#include "CamCapture.h"

#include <iostream>



// The xioctl method was a copied method from the v4l example code mentioned in the wiki
// Considering depricating this in this class.
static int
xioctl                          (int                    fd,
                                 int                    request,
                                 void *                 arg)
{
        int r;

        do r = ioctl (fd, request, arg);
        while (-1 == r);

        return r;
}




CamCapture::CamCapture() {
	// TODO Auto-generated constructor stub

}

CamCapture::CamCapture(char* sDev){
	//std::cout << "!!!Hello ddd World!!!" << sDev << std::endl;
	m_sDevicePath = sDev;
	m_bDeviceOpen = false;
	m_bDeviceInitd = false;
	m_bDeviceCapturing = false;
	m_ptrBuffers = NULL;
	m_pSwsContext = NULL;
}

int CamCapture::openDevice(){
	/*
	 * This will open the Device path and get the File Descriptor.
	 * return 0 for success and -1 for failure.
	 */
	 //struct stat statCtx;
	 m_fileDescriptor = open(m_sDevicePath, O_RDWR /* required */ | O_NONBLOCK, 0);

	 if (-1 == m_fileDescriptor) {
	                 //std::cerr << "Cannot open '"<< m_sDevicePath <<std::endl;
	                 //exit (EXIT_FAILURE);
		 return -1;
	         }
	 m_bDeviceOpen = true;
	 return 0;


}
int CamCapture::closeDevice(){
	/*
	 * Will close the device, the device must already be uninitilised for this
	 * will return a negative number if fail 0 for success.
	 */
	int errnum = close(m_fileDescriptor);
	if (errnum == -1)
		return -1;

	m_bDeviceOpen = false;
	return 0;
}

unsigned long int CamCapture::capabilities(){
	/*
	 * V4L2 spec says this *must* be called before anything else
	 * happens. i.e. right after the device is opened.
	 */
	if (!m_bDeviceOpen)
		return 0;

	struct v4l2_capability capCtx;
	xioctl (m_fileDescriptor, VIDIOC_QUERYCAP, &capCtx);

	return capCtx.capabilities;

}

int CamCapture::initDevice(){
	/*
	 * Tells the device what format we require the images to be passed in and what size
	 * and type. As well as intialise all the buffers for MMAP mode.
	 */
	struct 			v4l2_cropcap cropcap;
	struct 			v4l2_crop crop;
    struct 			v4l2_format fmt;
    struct 			v4l2_requestbuffers req;
    unsigned int	iBuffer;
    unsigned int 	min;

	memset( &cropcap,  0,  sizeof( struct v4l2_cropcap ) );
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl( m_fileDescriptor,  VIDIOC_CROPCAP,  &cropcap );

	// Cropping not supported on device, there is no need to anyway.
	/*crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect; // reset to default
	xioctl (m_fileDevice, VIDIOC_S_CROP, &crop);*/

	memset( &fmt,  0,  sizeof(struct v4l2_format) );

	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = 640;
	fmt.fmt.pix.height      = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	 /* Buggy driver paranoia. */
	 min = fmt.fmt.pix.width * 2;
	 if (fmt.fmt.pix.bytesperline < min)
		 fmt.fmt.pix.bytesperline = min;

	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	//init_memorymap
	memset (&req, 0, sizeof (struct v4l2_requestbuffers) );
	req.count               = 4;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	xioctl ( m_fileDescriptor, VIDIOC_REQBUFS, &req );

	 m_ptrBuffers = (struct buffer*) calloc (req.count, sizeof( *m_ptrBuffers ) );

	 //WTF to this For loop, init buffers?
	 for ( iBuffer = 0; iBuffer < req.count; ++iBuffer ){ //Do I need to look in the difference btwn ++i and i++?
		 struct v4l2_buffer buf;

		 memset( &buf, 0, sizeof( struct v4l2_buffer ) );

		 buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		 buf.memory = V4L2_MEMORY_MMAP;
		 buf.index = iBuffer;

		 xioctl( m_fileDescriptor, VIDIOC_QUERYBUF, &buf );

		 m_ptrBuffers[ iBuffer ].length = buf.length;
		 m_ptrBuffers[ iBuffer ].start =
		 mmap(NULL /* start anywhere */,
                 buf.length,
                 PROT_READ | PROT_WRITE /* required */,
                 MAP_SHARED /* recommended */,
                 m_fileDescriptor, buf.m.offset);

	 }

	 m_uBufferCount = req.count;
	 m_bDeviceInitd = true;


	return 0;
}

int CamCapture::uninitDevice(){
	/*
	 * This unitilises all the buffers with munmap
	 *
	 */
	unsigned int iBuffer;

	for ( iBuffer = 0 ; iBuffer < m_uBufferCount; iBuffer++)
	{
		munmap(m_ptrBuffers[iBuffer].start, m_ptrBuffers[iBuffer].length);
	}
	free ( m_ptrBuffers );
	m_bDeviceInitd = false;

	return 0;
}

bool CamCapture::isInitd(){
	/*
	 * Is the device initialised?
	 */
	return m_bDeviceInitd;
}

bool CamCapture::isOpen(){
	/*
	 * is the device open?
	 */
	return m_bDeviceOpen;
}

int CamCapture::startCapture(){
	/*
	 * This prepares the camera by queuing up all the buffers and telling
	 * the camera to start writing to them
	 */
	unsigned int iBuffer;
	enum v4l2_buf_type type;

	for (iBuffer = 0; iBuffer < m_uBufferCount; ++iBuffer) {
	       struct v4l2_buffer buf;

	       memset (&buf, 0, sizeof (struct v4l2_buffer) );

	       buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	       buf.memory      = V4L2_MEMORY_MMAP;
	       buf.index       = iBuffer;

	       xioctl (m_fileDescriptor, VIDIOC_QBUF, &buf);



	}

	m_ptrCamFrame = avcodec_alloc_frame();


	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(m_fileDescriptor, VIDIOC_STREAMON, &type);

	m_bDeviceCapturing = true;
	return 0;

} //startCapture()

int CamCapture::stopCapture(){
	/*
	 * Tells the camera to stop writing to the queued buffers
	 */
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl (m_fileDescriptor, VIDIOC_STREAMOFF, &type);
	m_bDeviceCapturing = false;

	free(m_ptrCamFrame);

	return 0;
}

bool CamCapture::isCapturing()
{
	return m_bDeviceCapturing;
}

int CamCapture::setConverstion(int width, int height, enum PixelFormat pixFormat)
{
	if(m_pSwsContext != NULL)
	{
		free(m_pSwsContext);
	}

	m_pSwsContext = sws_getContext(640, 480,
			PIX_FMT_YUYV422,
			width,  height, pixFormat, SWS_FAST_BILINEAR,
            NULL, NULL, NULL);

	m_nScalingHeight = height;

	return 0;


}
AVFrame* CamCapture::allocFrame(int width, int height, enum PixelFormat pixFormat){
	/*
	 * This will allocate the buffers for a suitable Frame that the cam will capture
	 * in. This will need to be free'delse where.
	 */
	AVFrame* pFrame;
	uint8_t* bufFrame;
	int bufsize;

	bufsize = avpicture_get_size(PIX_FMT_YUV420P, width, height);
	bufFrame = (uint8_t*)av_malloc(bufsize);
	pFrame = avcodec_alloc_frame();

	avpicture_fill((AVPicture *)pFrame, (uint8_t*)bufFrame, pixFormat,
				width, height);

	return pFrame;


}

int CamCapture::grabPicture(AVFrame* picture)
{
	//picture->
	fd_set fds;
	struct timeval tv;
	struct v4l2_buffer buf;
	//struct AVFrame*	camFrame;

	FD_ZERO( &fds );
	FD_SET( m_fileDescriptor, &fds );

	/* Timeout. */
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	select( m_fileDescriptor+1, &fds, NULL, NULL, &tv );

	memset( &buf, 0, sizeof (struct v4l2_buffer) );

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	xioctl (m_fileDescriptor, VIDIOC_DQBUF, &buf); //LOCK (i think)

	//Do your magic!

	avpicture_fill((AVPicture *)m_ptrCamFrame, (uint8_t*) m_ptrBuffers[buf.index].start, PIX_FMT_YUYV422,
							640, 480);
	//m_ptrCamFrame->data[0] = (uint8_t*) m_ptrBuffers[buf.index].start;

	if(m_pSwsContext != NULL)
	{
		sws_scale(m_pSwsContext,
				m_ptrCamFrame->data,
				m_ptrCamFrame->linesize,
				0,
				480,
				//m_nScalingHeight,
				picture->data,
				picture->linesize);
	}

	xioctl (m_fileDescriptor, VIDIOC_QBUF, &buf); //UNLOCK (i think)


	return 0;
}

CamCapture::~CamCapture() {
	// TODO Auto-generated destructor stub


}
