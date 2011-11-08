//============================================================================
// Name        : v4l-cpp-sandbox.cpp
// Author      : 
// Version     :
// Copyright   : LEE WAS ERE
// Description : Hello World in C++, Ansi-style
//============================================================================




//Bug? with INT64_C not being decalred in scope. Silly C++
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif //ifndef INT64_C

#include <iostream>
#include "VideoCompressor.h"
extern "C" {
#include <time.h>
#include <libavformat/avformat.h>
#include <libavutil/log.h>
//#include <libavformat/avio.h>
//#include "ffmpeginternal/libavformat/mpegenc.c"
}
#include "CamCapture.h"
#include "NetCommander.h"
#include "HTTPStreamer.h"
#include "HTTPServer.h"
#include "IOBuffer.h"
#include "FileDump.h"
#include "UDPTransportServer.h"
#include "Packetiser.h"

#include "TestThread.h"
#include <vector>
#include <string.h>
using namespace std;


struct encoderSupport
{
	enum CodecID codecID;
	string sName;
	bool bCanFind;
	bool bCanLoad;
};

struct argSettings
{
	bool bParsedCorrectly;
	bool bShowPacketisers;
	bool bShowCodecs;
	bool bShowHelp;
	char* sVideoDev;
	int nSendWidth;
	int nSendHeight;
	double nTimeLimit; // time limit when to close program.
	char* sPacketiser;
	bool bUseGlobalHeaders;
	bool bMP4HeaderHack;
	enum CodecID codecID;

	bool bPrintTimeStamps;

	bool bHTTPServer;
	int  nHTTPPort;
	bool bFileDump;
	char* sFileDump;


};


//GLOBALS! (I know, I'm a bad bad programmer </3 )
/*
struct sockaddr_in serv_addr, cli_addr;
int sockfd, cliLen;
int portno = 5125;
socklen_t fromlen;
char* socbuf;
*/

vector<struct encoderSupport*> *GEncoderList = NULL;


void populateEncoderSupport();

void parseArgs(int argv, char** argc, struct argSettings* set)
{
	//Set default values first!
	set->bParsedCorrectly = false;
	set->sVideoDev = NULL;
	set->bShowPacketisers = false;
	set->bShowCodecs = false;
	set->bShowHelp = false;
	set->nSendHeight = 120;
	set->nSendWidth = 180;
	set->codecID = CODEC_ID_MPEG4;
	set->bHTTPServer = true;
	set->nHTTPPort = 8080;
	set->sPacketiser = "mpegts";
	set->bFileDump = false;
	set->sFileDump = NULL;
	set->bUseGlobalHeaders = true; //Because MPEG4 codec is default!
	set->bPrintTimeStamps = false;
	set->nTimeLimit = 0; // 0 = Infinity
	set->bMP4HeaderHack = false;

	if(argv < 2)
		return;

	int i;
	for(i=1; i<argv; i++)
	{
		if( strcmp(argc[i], "-pp") == 0 )
		{
			set->bShowPacketisers = true;
			continue;
		}

		if( strcmp(argc[i], "-pc") == 0 )
		{
			set->bShowCodecs = true;
			continue;
		}

		if( strcmp(argc[i], "-help") == 0 )
		{
			set->bShowHelp = true;
			continue;
		}

		if( strcmp(argc[i], "-w") == 0 ) // set width
		{
			if(argv <= i+1) //is the next arg specified?
			{
				cout << "Did not specify width amount" << endl;
				return;
			}

			int width = atoi(argc[i+1]);
			if(width <= 0)
			{
				cout << argc[i] << " is an invalid width value" << endl;
				return;
			}

			set->nSendWidth = width;
			i++;
			continue;

		} // -w

		if( strcmp(argc[i], "-h") == 0 ) // set height
		{
			if(argv <= i+1) //is the next arg specified?
			{
				cout << "Did not specify height amount" << endl;
				return;
			}

			int height = atoi(argc[i+1]);
			if(height <= 0)
			{
				cout << argc[i] << " is an invalid height value" << endl;
				return;
			}

			set->nSendHeight = height;
			i++;
			continue;

		} // -h

		if( strcmp(argc[i], "-codec") == 0 ) // set codec
		{
			if(argv <= i+1) //is the next arg specified?
			{
				cout << "Did not specify codec name" << endl;
				return;
			}

			char* codecName = argc[i+1];
			string sCodecName(codecName);

			//Scan list for it!
			populateEncoderSupport();
			bool isFound = false;
			vector<struct encoderSupport*>::iterator itEncoder;

			for(itEncoder = GEncoderList->begin(); itEncoder != GEncoderList->end(); itEncoder++)
			{

				if( (*itEncoder)->sName.compare(0, (*itEncoder)->sName.length(), sCodecName) == 0)
				{
					isFound = true;
					set->codecID = (*itEncoder)->codecID;

					if((*itEncoder)->codecID == CODEC_ID_MPEG4) //Just the way MP4 works...
						set->bUseGlobalHeaders = true;
					else
						set->bUseGlobalHeaders = false;
					break; // break from scanning encoder list
				}
			}//Scanning through encoder list

			if(!isFound)
			{
				cout << "Could not find codec: " << codecName <<
						". Use -pc to find supported codecs (case sensitive)"<< endl;
				return;

			}

			i++;
			continue;


		} //codec


		if( strcmp(argc[i], "-pack") == 0 ) // set packitiser
		{
			if(argv <= i+1) //is the next arg specified?
			{
				cout << "Did not specify packetiser name" << endl;
				return;
			}

			set->sPacketiser = argc[i+1];
			i++;
			continue;


		}

		if( strcmp(argc[i], "-t") == 0 ) // set timelimit
		{
			if(argv <= i+1) //is the next arg specified?
			{
				cout << "Did not specify time limit amount" << endl;
				return;
			}

			set->nTimeLimit = atof(argc[i+1]);
			i++;
			continue;

		}

		if( strcmp(argc[i], "-o") == 0 ) // set file output
		{
			if(argv <= i+1) //is the next arg specified?
			{
				cout << "Did not specify where to write file" << endl;
				return;
			}

			set->bFileDump = true;
			set->sFileDump = argc[i+1];
			i++;
			continue;
		}// -o file output

		if( strcmp(argc[i], "-pts") == 0 ) // printtimestamp
		{
			set->bPrintTimeStamps = true;
			continue;
		}

		if( strcmp(argc[i], "-hgh") == 0 ) // hack-global-header-enable
		{
			set->bUseGlobalHeaders = true;
			continue;
		}


		if( strcmp(argc[i], "-hmp4header") == 0 ) // hack-mp4-header-at-start-of-stream
		{
			set->bMP4HeaderHack = true;
			continue;
		}

		if(argv == i+1) //at the end, must be video Device, if all above fails
		{
			set->sVideoDev = argc[i];
			continue;
		}

		//Okay, unrecognised argument :(
		cout << "Unrecognised argument: " << argc[i] << endl;
		return;

	}//for i

	//Didn't return; at any errors, must of been successful!
	set->bParsedCorrectly = true;

}

void printHelp()
{
	/*
	 * This will print off a little mini help list :)
	 */

	cout << "./camSvr [args] [Path-To-Video-Dev]" << endl;
	cout << endl;
	cout << "  -w     [int]        Set the width of encoded video" << endl;
	cout << "  -h     [int]        Set the height of encoded video" << endl;
	cout << "  -codec [codec]      Set the codec to use" << endl;
	cout << "  -pack  [packetiser] Set the packetiser to use" << endl;
	cout << "  -t     [seconds]    Set the number of seconds for the program to close" << endl;
	cout << "  -o     [path]       Write file for compressed stream" << endl;
	cout << endl;
	cout << "  -pts                Print out time-stamps per frame" << endl;
	cout << endl;
	cout << "  -hgh                Enable Global Headers, needed for some Containers" << endl;
	cout << "  -hmp4header         Enable mp4 header to be at the start of the stream" << endl;
	cout << endl;
	cout << "  -help               Print this help" << endl;
	cout << "  -pp                 Print packetiser Support" << endl;
	cout << "  -pc                 Print codec Support" << endl;
}




void testPacketiserSupport()
{
	AVOutputFormat *format;
	format = NULL;
	while(1){
			format = av_oformat_next(format);
			if(!format) break;
			printf("%s %s\n", format->name, format->mime_type);

		}
	return;
}

void populateEncoderSupport(){
	/*
	 * This will run try to set up a number of encoders and try and say if they can be loaded in
	 * or not using our class. and print the results.
	 */

	if(GEncoderList == NULL)
		GEncoderList = new vector<struct encoderSupport*>();
	else
		return;

	struct encoderSupport* encoder;

	encoder = new struct encoderSupport;
	encoder->codecID = CODEC_ID_MPEG2VIDEO;
	encoder->sName = "MPEG2";
	GEncoderList->push_back(encoder);

	encoder = new struct encoderSupport;
	encoder->codecID = CODEC_ID_VP3;
	encoder->sName = "VP3";
	GEncoderList->push_back(encoder);

	encoder = new struct encoderSupport;
	encoder->codecID = CODEC_ID_H261;
	encoder->sName = "h.261";
	GEncoderList->push_back(encoder);

	encoder = new struct encoderSupport;
	encoder->codecID = CODEC_ID_H263;
	encoder->sName = "h.263";
	GEncoderList->push_back(encoder);

	encoder = new struct encoderSupport;
	encoder->codecID = CODEC_ID_MPEG4;
	encoder->sName = "MPEG4";
	GEncoderList->push_back(encoder);

	encoder = new struct encoderSupport;
	encoder->codecID = CODEC_ID_MJPEG;
	encoder->sName = "MJPEG";
	GEncoderList->push_back(encoder);

	encoder = new struct encoderSupport;
	encoder->codecID = CODEC_ID_H264;
	encoder->sName = "h.264";
	GEncoderList->push_back(encoder);

	encoder = new struct encoderSupport;
	encoder->codecID = CODEC_ID_THEORA;
	encoder->sName = "THEORA";
	GEncoderList->push_back(encoder);

	encoder = new struct encoderSupport;
	encoder->codecID = CODEC_ID_MPEG1VIDEO;
	encoder->sName = "MPEG1";
	GEncoderList->push_back(encoder);
}

void testEncoderSupport(){

	if(GEncoderList == NULL)
		populateEncoderSupport();

	vector<struct encoderSupport*>::iterator itEncoder;
	VideoCompressor *comp;
	int errornum;
	for(itEncoder = GEncoderList->begin(); itEncoder != GEncoderList->end(); itEncoder++)
	{
		comp = new VideoCompressor();
		comp->setCodecID((*itEncoder)->codecID);

		if((*itEncoder)->codecID == CODEC_ID_MJPEG)
		{
			comp->setColourSpace(PIX_FMT_YUVJ420P); // PIX_FMT_YUVJ420P + PIX_FMT_YUVJ422P
		}
		errornum = comp->setupEncoder();
		if(errornum == -1) //Failed to locate encoder
		{
			(*itEncoder)->bCanFind = false;
			(*itEncoder)->bCanLoad = false;
		}
		if(errornum == -2) //Failed to load CodecContext + Encoder (After it found the encoder!)
		{
			(*itEncoder)->bCanFind = true;
			(*itEncoder)->bCanLoad = false;
		}

		if(errornum == 0) // No errors!
		{
			(*itEncoder)->bCanFind = true;
			(*itEncoder)->bCanLoad = true;
		}

		delete comp;
	}

	//print results
	cout << "Encoder Found? Loaded?" << endl;
	for(itEncoder = GEncoderList->begin(); itEncoder != GEncoderList->end(); itEncoder++)
	{
		cout << (*itEncoder)->sName << " " << (*itEncoder)->bCanFind << " " << (*itEncoder)->bCanLoad << endl;
	}



}


int main(int argv, char** argc) {
	cout << "CamSvr..." << endl;
	av_log_set_level(AV_LOG_DEBUG);
	//char device[] = "/dev/video0";
	CamCapture *cammy;
	VideoCompressor *encoder;
	FileDump* fileDumper = NULL;
	FileDump* fileDumperDEBUG = NULL;
	av_register_all();

	struct argSettings settings;
	parseArgs(argv, argc, &settings);

	//JPEG TESTING HERE
	/*
	VideoCompressor *encoderJPEG = NULL;
	encoderJPEG = new VideoCompressor();
	encoderJPEG->setWidthHeight(settings.nSendWidth, settings.nSendHeight);
	encoderJPEG->setBitRate(128000);
	encoderJPEG->setCodecID( CODEC_ID_MJPEG);
	encoderJPEG->setColourSpace(PIX_FMT_YUVJ420P);
	encoderJPEG->setGlobalHeaders(settings.bUseGlobalHeaders);
	FileDump* fileDumperJPEG = NULL;

	if(encoderJPEG->setupEncoder() != 0)
		{
			cout << "Error Setting up the JPEG Encoder, EXITING" << endl;
			exit(-1);
		}
	fileDumperJPEG = new FileDump();
	fileDumperJPEG->saveTo("/tmp/picture.jpg");
	*/

	if(!settings.bParsedCorrectly)
	{
		cout << "Could not parse args. EXITING" << endl;
		exit(0);
	}

	if(settings.bShowHelp)
	{
		printHelp();
		exit(0);
	}
	if(settings.bShowCodecs)
	{
		testEncoderSupport();
		exit(0);
	}
	if(settings.bShowPacketisers)
	{
		testPacketiserSupport();
		exit(0);
	}
	if(settings.bMP4HeaderHack)
	{
		cout << "MP4 Header Hack Enabled" << endl;
	}

	if(settings.bFileDump)
	{
		fileDumper = new FileDump();
		if(fileDumper->saveTo(settings.sFileDump) != 0)
		{
			cout << "Could not write to: " << settings.sFileDump << " EXITING" <<endl;
			exit(0);
		}
		cout << "Writing to: " << settings.sFileDump << endl;

		//DEBUG DELETE ME AFTER USE :(
		fileDumperDEBUG = new FileDump();
		fileDumperDEBUG->saveTo("/tmp/dbg.mpg");
	}

	//Setup the Encoder!
	cout << "Setting up Encoder!" << endl;
	encoder = new VideoCompressor();
	encoder->setWidthHeight(settings.nSendWidth, settings.nSendHeight);
	encoder->setBitRate(128000);
	encoder->setCodecID(settings.codecID);
	if(settings.codecID == CODEC_ID_MJPEG)
	{
		encoder->setColourSpace(PIX_FMT_YUVJ420P);
	}
	encoder->setGlobalHeaders(settings.bUseGlobalHeaders);

	if(encoder->setupEncoder() != 0)
	{
		cout << "Error Setting up the Encoder, EXITING" << endl;
		exit(-1);
	}
	cout << "Encoder setup successfully! Codec: " << encoder->getCodecContext()->codec->name << endl;

	//Alloc Buffers for passign raw Encoded Data between it and the packetiser, needed later.
	char* encodeBuffer;
	int encodeBufferSize = 65535;
	encodeBuffer = (char*)malloc( encodeBufferSize+1 );

	//Alloc Our Special Super Duper ByteIO context :)
	IOBuffer * ioBuffer = new IOBuffer();

	//TODO: Setup up Packetiser class once implemented...
	Packetiser* mrPacket = new Packetiser( settings.sPacketiser );
	mrPacket->addVideoStream( encoder->getCodecContext() );
	mrPacket->setByteIOContext( ioBuffer->getByteIOContext() );
	mrPacket->setMP4HeaderHack(settings.bMP4HeaderHack);
	if ( mrPacket->initPacketiser() != 0)
	{
		cout << "Failed to Init Packetiser. EXITING" << endl;
		exit(-1);
	}

	if(settings.bMP4HeaderHack)
	{
		//Copy the ioBufferFirst :)
		ioBuffer->copyPresentBufferToFirst();
		cout << "Copied that B#@tch up" << endl;
	}




	// Init the Camera!
	cammy = new CamCapture(settings.sVideoDev);
	AVFrame* pFrame;
	pFrame = cammy->allocFrame(settings.nSendWidth, settings.nSendHeight, PIX_FMT_YUV420P);

	cammy->setConverstion(settings.nSendWidth, settings.nSendHeight, PIX_FMT_YUV420P);
	cammy->openDevice();
	if(cammy->isOpen()){
		cout << "Camera Device Opened!" << endl;
	}else{
		cout << "Failed To Access Camera. Are you sure it is at: " << settings.sVideoDev << endl;
		exit(-1);
	}

	//unsigned long int nCap =
			cammy->capabilities(); // should be 0x04000001 with c200 cam
	//char* sCap = (char*) &nCap;
	//cout << "Caps " << sCap[0] << sCap[1] << sCap[2] << sCap[3] << endl;

	cammy->initDevice();
	if(cammy->isInitd()){
		cout << "Device Init'd!" << endl;
	}else{
		cout << "Failed To Init?" << endl;
		exit(-1);
	}


	cammy->startCapture();
	if( cammy->isCapturing() )
	{
		cout << "Device Capturing!" << endl;
	}else{
		cout << "Failed To capture with WebCam" << endl;
		return -1;
	}

	//Create the HTTPServer
	HTTPServer* httpserver = new HTTPServer();
	httpserver->Start();
	cout << "HTTP Server Started" << endl;

	//Create the HTTPServer with the Packetiser in it...
	HTTPServer* httpserverp = new HTTPServer();
	if(settings.bMP4HeaderHack)
	{
		ioBufferContext* ioctx = (ioBufferContext*)ioBuffer->getByteIOContext()->opaque;
		//uint8_t ** buffs, int* lengths, int size

		httpserverp->setFirstList(ioctx->firstPacketList, ioctx->firstPacketLength, ioctx->firstPacketListSize);
	}
	else
	{
		httpserverp->setUpPacketiser(settings.sPacketiser, encoder->getCodecContext());
	}
	httpserverp->setListenPort(8090);
	httpserverp->Start();
	cout << "HTTP Server2 Started" << endl;

	//Creater the UDPServer!
	UDPTransportServer* udpserver = new UDPTransportServer();
	udpserver->Start();
	cout << "UDP Server Started" << endl;

	//UDP broadcast server
	UDPTransportServer* udpBroadcast = new UDPTransportServer();
	udpBroadcast->setPort(5126);
	udpBroadcast->setBroadcast(true);
	cout << "UDP Broadcast Server Started" << endl;

	// CLEAN UP AFTER HERE :(
	int frameNo = 0;

	struct timeval tsProcessImage;
	struct timeval tsScaledImage;
	struct timeval tsEncodedImage;
	struct timeval tsSentImage;
	struct timeval tsStart;
	struct timeval tsNow;
	struct timedif tdScale;
	struct timedif tdEncode;
	struct timedif tdSend;
	struct timedif tdSecTS; // second TimeStamp
	struct timezone tz;



		 //DO NOT WRITE TO! THE GOGGLES, THEY DO NOTHING!
		 //video_stream->time_base.den = 30;
		 //video_stream->time_base.num = 1;





		  int ret;
		  AVPacket packet;

		  gettimeofday(&tsStart, NULL);


		  //Print out what we're doing?
		  cout << "Using codec: " << encoder->getCodecContext()->codec->name << " Packetising: " << mrPacket->getName() << endl;

	int bufsize;
	mrPacket->setTimeBase(1, 30);
	bool isIFrame = false;
    while(1)
	{

    	if(frameNo%10 == 0) isIFrame = true; else isIFrame = false;

		gettimeofday(&tsProcessImage, &tz);
		cammy->grabPicture(pFrame);
		gettimeofday(&tsScaledImage, &tz);


		encoder->encodeFrame(pFrame, encodeBuffer, encodeBufferSize, &bufsize);

		//encoderJPEG->encodeFrame(pFrame, encodeBuffer, encodeBufferSize, &bufsize);

		//fileDumperJPEG->sendData(encodeBuffer, bufsize);
		//exit(0);

		//UDP RAW!
		udpserver->sendData(encodeBuffer, bufsize);
		//cout << "sent to all " << endl;
		//udpBroadcast->sendData(encodeBuffer, bufsize);

		//cout << "Frame: " << frameNo << " Size:" << bufsize << " " << isIFrame << endl;

		//packet.dts = AV_NOPTS_VALUE;




		gettimeofday(&tsEncodedImage, &tz);

		mrPacket->setPacketStamps(&packet, &tsEncodedImage);
		if(settings.bPrintTimeStamps)
					cout << "pts: " << packet.pts << " dts: " << packet.dts <<endl;
		packet.data  = (uint8_t*)encodeBuffer;
		packet.size = bufsize;


		//Simple hax
		ioBufferContext* hehe = (ioBufferContext*)ioBuffer->getByteIOContext()->opaque;
		int rofl = hehe->writeIndex - 1;
		if (rofl >= hehe->packetListSize || rofl < 0)
				rofl = 0;


		av_dup_packet(&packet);
		if(!settings.bMP4HeaderHack)
			httpserverp->packitiseAndSend(&packet, isIFrame);
		else
		{

			httpserverp->sendData((char*)hehe->packetList[rofl], hehe->packetLength[rofl]); //Too Nerdy it hurts

		}
		//Real Slim Shady

		mrPacket->writeInPacket(&packet);

		frameNo++;
		//ss->sendPacket(encodeBuffer, bufsize);
		//httpserver->sendData(encodeBuffer, bufsize);





		httpserver->sendData((char*)hehe->packetList[rofl], hehe->packetLength[rofl]);
		if(settings.bFileDump)
		{
			//fileDumper->sendData((char*)hehe->packetList[rofl], hehe->packetLength[rofl]);
			fileDumper->writeBuffer((ioBufferContext*)ioBuffer->getByteIOContext()->opaque);
			//fileDumperDEBUG->sendData((char*)heheD->packetList[roflD], heheD->packetLength[roflD]);
		}




		if (settings.nTimeLimit != 0)
		{
			//THERE is a limit...

			//tsStart & tsEncodedImage
			struct timedif tdElapse;
			double tSec;
			timeDifCalc(&tsStart, &tsEncodedImage, &tdElapse);
			tSec = (double)(tdElapse.dsec) +  ((double)(tdElapse.dusec))*0.000001;
			//cout << "secs: " << tdElapse.dsec << " " << tSec << endl;
			if(settings.nTimeLimit < tSec )
			{
				break;
			}

		}// if time limit set
	}// while 1

	usleep(1000000);
	mrPacket->addTrailer();
	fileDumper->writeBuffer((ioBufferContext*)ioBuffer->getByteIOContext()->opaque);
	cammy->stopCapture();
	cammy->uninitDevice();



	cammy->closeDevice();
	if(!cammy->isOpen()){
			cout << "Device Closed!" << endl;
		}else{
			cout << "Failed To close?" << endl;
			return -1;
		}


	return 0;
}
