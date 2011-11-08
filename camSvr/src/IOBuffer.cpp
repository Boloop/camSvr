/*
 * IOBuffer.cpp
 *
 *  Created on: 29 Mar 2011
 *      Author: two
 */

#include "IOBuffer.h"
#include <iostream>
using namespace std;



int ioBufferWriteFunc(void *opaque, uint8_t *buf, int buf_size)
{
	/*
	 * Copy and write the buffer into the list!
	 */
	ioBufferContext* bufcon = (ioBufferContext*)opaque;
	uint8_t* bufcopy;
	//Copy the buffer
	bufcopy = (uint8_t*)malloc(buf_size);
	memcpy(bufcopy, buf, buf_size);

	pthread_mutex_lock( &(bufcon->lock) );

	//Check where we are to write next, reset to 0 if reached end
	if(bufcon->writeIndex >= bufcon->packetListSize)
	{
		//Reset to 0
		bufcon->writeIndex = 0;
	}

	//Free up the memory that's allocated
	if (bufcon->packetList[bufcon->writeIndex] != NULL)
	{
		//It's not null, Free
		free(bufcon->packetList[bufcon->writeIndex]);
	}

	//Write over the pointer! :)
	bufcon->packetList[bufcon->writeIndex] = bufcopy;
	bufcon->packetLength[bufcon->writeIndex] = buf_size;


	if(bufcon->currentPos == bufcon->totalWritten)
	{
		//at the end!
		bufcon->packetPosition[bufcon->writeIndex] = -1;
	} else {
		//not at the end :(
		bufcon->packetPosition[bufcon->writeIndex] = bufcon->currentPos;
	}

	bufcon->writeIndex = 1 + bufcon->writeIndex;

	//Is this the first set?
	if(bufcon->firstWriteEnabled && bufcon->firstWriteIndex < bufcon->firstPacketListSize)
	{
		bufcopy = (uint8_t*)malloc(buf_size);
		memcpy(bufcopy, buf, buf_size);

		bufcon->firstPacketList[bufcon->firstWriteIndex] = bufcopy;
		bufcon->firstPacketLength[bufcon->firstWriteIndex] = buf_size;
		bufcon->writeIndex += 1;
	}

	bufcon->currentPos += buf_size;
	if (bufcon->currentPos > bufcon->totalWritten)
		bufcon->totalWritten = bufcon->currentPos;

	pthread_mutex_unlock( &(bufcon->lock) );






	return 0;
}

int ioBufferReadFunc(void *opaque, uint8_t *buf, int buf_size)
{
	return 0;
}

int64_t ioBufferSeekFunc(void *opaque, int64_t offset, int whence)
{
	/*
	 * this is mostly used to debug. Not real interesting stuff here :(
	 */
	ioBufferContext* bufcon = (ioBufferContext*)opaque;
	cout << "Seek W:"<< whence << " offset: " << offset << endl;
	if(SEEK_SET == whence)
		cout << "SEEKSET" << endl;
	if(SEEK_CUR == whence)
			cout << "SEEKCUR" << endl;
	if(SEEK_END == whence)
			cout << "SEEKEND" << endl;


	pthread_mutex_lock( &(bufcon->lock) );
	bufcon->printWrites = 0x01; // true!
	if(SEEK_SET == whence)
	{
		bufcon->currentPos = offset;
	}

	pthread_mutex_unlock( &(bufcon->lock) );
	return 0;
}



IOBuffer::IOBuffer()
{
	// TODO Auto-generated constructor stub
	m_pByteIOContext = NULL;
	m_nBufferSize = 1024*1024*8; //8 MB
	m_pBuffer = (uint8_t*) malloc(m_nBufferSize);


	m_pIOBufferContext = (ioBufferContext*) malloc(sizeof(ioBufferContext)); //Alloc IOBufferContext

	//Initilise the IOBufferContext :)
	m_pIOBufferContext->packetListSize = 100; // insane! D:
	m_pIOBufferContext->packetList = (uint8_t**) malloc( sizeof(uint8_t*) * m_pIOBufferContext->packetListSize );
	m_pIOBufferContext->packetLength = (int*) malloc( sizeof(int) * m_pIOBufferContext->packetListSize );
	m_pIOBufferContext->packetPosition = (int*) malloc( sizeof(int) * m_pIOBufferContext->packetListSize );
	m_pIOBufferContext->writeIndex = 0;
	m_pIOBufferContext->lock = PTHREAD_MUTEX_INITIALIZER;

	//Fill up packet lists with NULL/0
	for(int i=0; i< m_pIOBufferContext->packetListSize; i++)
	{
		m_pIOBufferContext->packetList[i] = NULL;
		m_pIOBufferContext->packetLength[i] = 0;
	}


	//Going to store first 20 packets!
	m_pIOBufferContext->firstPacketListSize = 10;
	m_pIOBufferContext->firstPacketList = (uint8_t**) malloc( sizeof(uint8_t*) * m_pIOBufferContext->firstPacketListSize );
	m_pIOBufferContext->firstPacketLength = (int*) malloc( sizeof(int) * m_pIOBufferContext->firstPacketListSize );
	m_pIOBufferContext->firstWriteEnabled = 0;
	m_pIOBufferContext->firstWriteIndex = 0;

	m_pIOBufferContext->totalWritten = 0;
	m_pIOBufferContext->currentPos = 0;
	m_pIOBufferContext->printWrites = 0x00; // false.

	m_pByteIOContext = av_alloc_put_byte(m_pBuffer, m_nBufferSize, 1, (void*)m_pIOBufferContext, ioBufferReadFunc, ioBufferWriteFunc, ioBufferSeekFunc);

/*
	init_put_byte(	ByteIOContext *	s,
	unsigned char *	buffer,
	int	buffer_size,
	int	write_flag,
	void *	opaque,
	int(*)(void *opaque, uint8_t *buf, int buf_size)	read_packet,
	int(*)(void *opaque, uint8_t *buf, int buf_size)	write_packet,
	int64_t(*)(void *opaque, int64_t offset, int whence)	seek
	)
	*/

}

ByteIOContext * IOBuffer::getByteIOContext()
{
	return m_pByteIOContext;
}

void IOBuffer::copyPresentBufferToFirst()
{
	/*
	 * Does what it says on the tin. Copies all the buffers
	 * and replaces them into the first one.
	 */

	//No freeing of current stuff, Shouldn't be allocated and shouldn't
	// auto fill to end.

	int i;
	int size = m_pIOBufferContext->writeIndex -1;
	if(size < 0) size = m_pIOBufferContext->packetListSize;
	size += 1;

	//Allocate the pointer list :)
	m_pIOBufferContext->firstPacketList = (uint8_t**)malloc(size*sizeof(uint8_t*));
	m_pIOBufferContext->firstPacketLength = (int*)malloc(size*sizeof(int));

	//Fill them up
	for(i = 0; i< size ;i++)
	{
		uint8_t * buf = (uint8_t*)malloc(m_pIOBufferContext->packetLength[i]);
		memcpy(buf, m_pIOBufferContext->packetList[i], m_pIOBufferContext->packetLength[i]);

		m_pIOBufferContext->firstPacketList[i] = buf;
		m_pIOBufferContext->firstPacketLength[i] = m_pIOBufferContext->packetLength[i];
	}

	m_pIOBufferContext->firstPacketListSize = size;
}

IOBuffer::~IOBuffer()
{
	// TODO Auto-generated destructor stub
}


/*
 *
 * IOBufferReader!
 *
 */

IOBufferReader::IOBufferReader(ioBufferContext * ioBufCon)
{
	/*
	 * Will set up the reading index as the latest packet
	 */
	m_pIOBufferContext = ioBufCon;
	pthread_mutex_lock( &(m_pIOBufferContext->lock) );

	m_nReadIndex = m_pIOBufferContext->writeIndex - 1;

	pthread_mutex_unlock( &(m_pIOBufferContext->lock) );

}

IOBufferReader::~IOBufferReader()
{

}
