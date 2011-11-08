/*
 * TestThread.h
 *
 *  Created on: 28 Feb 2011
 *      Author: two
 */

#ifndef TESTTHREAD_H_
#define TESTTHREAD_H_

#include "MyThread.h"
#include <iostream>
#include <time.h>

class TestThread: public MyThread {

private:
	char* m_sMsg;
public:
	TestThread(char* message);
	void Setup();
	void Execute();
	virtual ~TestThread();
};

#endif /* TESTTHREAD_H_ */
