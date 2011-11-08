/*
 * IOBuffer.h
 *
 *  Created on: 29 Mar 2011
 *      Author: two
 *
 *      This will be a class that will implement it's own ByteIOContext used for
 *      where the output AVFormatContext will write our MPEG-TS stream.
 *
 *      This was done mostly following this little blog here:
 *      http://cdry.wordpress.com/2009/09/09/using-custom-io-callbacks-with-ffmpeg/
 *
 */



#ifndef IOBUFFER_H_
#define IOBUFFER_H_

//Bug? with INT64_C not being decalred in scope. Silly C++
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif //ifndef INT64_C

extern "C" {
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libavcodec/avcodec.h>
	#include <libavformat/avio.h>
	#include <pthread.h>
}


typedef struct {
	uint8_t **   	packetList;
	int		*		packetLength;
	int		*		packetPosition;
	int    			packetListSize;
	int				writeIndex;
	pthread_mutex_t lock;

	int				firstPacketListSize;
	uint8_t **   	firstPacketList;
	int		*		firstPacketLength;
	int				firstWriteIndex;
	int				firstWriteEnabled; // True for storing first packets, false otherwise...
	int				totalWritten;
	int				currentPos;
	char			printWrites;



} ioBufferContext;


//Prototypes for the C read/write/seek functions.
int 	ioBufferWriteFunc(void *opaque, uint8_t *buf, int buf_size);
int 	ioBufferReadFunc(void *opaque, uint8_t *buf, int buf_size);
int64_t ioBufferSeekFunc(void *opaque, int64_t offset, int whence) ;


class IOBuffer
{
private:
	ByteIOContext * 	m_pByteIOContext;
	uint8_t *			m_pBuffer;
	int					m_nBufferSize;
	ioBufferContext *	m_pIOBufferContext;


public:
	IOBuffer();
	//IOBuffer(ioBufferContext* ctx);
	ByteIOContext * getByteIOContext();

	void copyPresentBufferToFirst();
	virtual ~IOBuffer();
};


/*
 * Yes, This class has Two classes in it, makes it easier for me IMHO
 * On second thoughts, No. This classed isn't implemented anywhere.
 * Left here to learn from historic failures.
 */

class IOBufferReader
{
private:

	int					m_nReadIndex;
	ioBufferContext *	m_pIOBufferContext;


public:
	IOBufferReader(ioBufferContext * ioBufCon);

	virtual ~IOBufferReader();
};


#endif /* IOBUFFER_H_ */
