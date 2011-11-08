/*
 * TestThread.cpp
 *
 *  Created on: 28 Feb 2011
 *      Author: two
 */

#include "TestThread.h"

TestThread::TestThread(char* message) {
	// TODO Auto-generated constructor stub
	m_sMsg = message;
}

void TestThread::Setup()
{
	std::cout << "Setup: " << m_sMsg << std::endl;
}

void TestThread::Execute()
{
	std::cout << "Start: " << m_sMsg << std::endl;
	usleep(1000000);
	std::cout << "Stop: " << m_sMsg << std::endl;
}


TestThread::~TestThread() {
	// TODO Auto-generated destructor stub
}
