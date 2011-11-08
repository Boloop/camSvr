/*
 * FileDump.cpp
 *
 *  Created on: 4 Apr 2011
 *      Author: two
 */

#include "FileDump.h"

FileDump::FileDump()
{
	// TODO Auto-generated constructor stub
	m_File = NULL;
	m_sFilePath = NULL;
	m_nBufferPosition = 0;

}

int FileDump::saveTo(char* fileName)
{
	/*
	 * This will setup the File ready to write
	 * return 0 for success, -1 for failure
	 */

	m_File = fopen(fileName, "wb");
	if (!m_File)
	{
		return -1;
	}
	m_sFilePath = fileName;
	return 0;
}

void FileDump::sendData(char *buffer, int length)
{
	if(!m_File)
		return;

	fwrite(buffer, 1, length, m_File);


}

void FileDump::writeBuffer(ioBufferContext* ctx)
{
	/*
	 * This will read an ioContext and dump all the data since it was last read to the file.
	 * This supports seeking, by checking the packcetPosition aspect of the buffer.
	 * Useful mostly for seeking, and writing several packets at once.
	 */
	int cycleNo;
	int i, j;

	pthread_mutex_lock(&(ctx->lock));
	if(ctx->writeIndex == m_nBufferPosition)
	{
		//Nothing to write
		pthread_mutex_unlock(&(ctx->lock));
		return;
	}


	cycleNo = ctx->writeIndex - m_nBufferPosition;
	if(cycleNo < 0)
		cycleNo = ctx->packetListSize + cycleNo;

	j = m_nBufferPosition;
	for(i=0; i<cycleNo; i++)
	{

		if(j >= ctx->packetListSize)
			j = j - ctx->packetListSize;

		if (ctx->packetPosition[j] < 0 )
		{
			//append to end!
			fseek(m_File, 0, SEEK_END);
			fwrite(ctx->packetList[j], 1, ctx->packetLength[j], m_File);

		}
		else
		{
			//seek to position
			fseek(m_File, ctx->packetPosition[j], SEEK_SET);
			cout << "Writing: " << ctx->packetLength[j] << " bytes at " << ctx->packetPosition[j] << endl;
			fwrite(ctx->packetList[j], 1, ctx->packetLength[j], m_File);
		}

		j++;

	}// for all the free food

	if(j >= ctx->packetListSize)
			j = j - ctx->packetListSize;

	m_nBufferPosition = j;
	pthread_mutex_unlock(&(ctx->lock));
	return;

}

FileDump::~FileDump()
{
	// TODO Auto-generated destructor stub
}
