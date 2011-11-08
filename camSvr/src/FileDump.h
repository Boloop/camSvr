/*
 * FileDump.h
 *
 *  Created on: 4 Apr 2011
 *      Author: two
 */

#ifndef FILEDUMP_H_
#define FILEDUMP_H_

#include <stdio.h>
#include <iostream>
#include "IOBuffer.h"

#include "TransporterService.h"

using namespace std;

class FileDump: public TransporterService
{
private:
	FILE* 	m_File;
	char* 	m_sFilePath;
	int		m_nBufferPosition;


public:
	FileDump();
	int saveTo(char* filePath);
	virtual ~FileDump();

	void writeBuffer(ioBufferContext* ctx);

	//Methods to override
	void sendData(char *buffer, int lenght);

};

#endif /* FILEDUMP_H_ */
