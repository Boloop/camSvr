/*
 * NetCommander.h
 *
 *  Created on: 2 Mar 2011
 *      Author: two
 *
 *      This Class is used to listen for a TCP connection to control the server with another application (such as python)
 *      This will be used to set parameters for the camera like what type to stream it to and where.
 */

#ifndef NETCOMMANDER_H_
#define NETCOMMANDER_H_

#include "MyThread.h"
extern "C" {
	#include <stdlib.h>
	#include <sys/socket.h>
	#include <sys/types.h>        /*  socket types              */
	#include <netinet/in.h>
	#include <arpa/inet.h>
//#include <sys/socket.h>       /*  socket definitions        */
//#include <sys/types.h>        /*  socket types              */
//#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */

}

#include <iostream>

class NetCommander: public MyThread {
private:
	int m_nPort;
	int m_fdSocket;
	struct sockaddr_in m_ServAddr;



public:
	NetCommander(int port);
	int bindSocket();
	void Setup();
	void Execute();
	virtual ~NetCommander();
};

#endif /* NETCOMMANDER_H_ */
