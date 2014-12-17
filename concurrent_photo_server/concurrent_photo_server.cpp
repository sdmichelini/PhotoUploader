//============================================================================
// Name        : concurrent_photo_server.cpp
// Author      : Stephen Michelini
// Version     :
// Copyright   : 
// Description : Concurrent Photo Server
//============================================================================

#include "data_link_layer.h"
#include "tcp_server.h"
#include <fstream>
#include <string>
#include <signal.h>

///Size of each read we will be making
#define READ_SIZE 200
//Global Variable for Server Finished
bool server_finished;
//Global TCP Server Object
tcp_server * s;

///Server listen port
#define LISTEN_PORT 36500

//Will finish upload and terminate
//Author: Stephen Michelini
void signal_callback_handler(int signum)

{

	printf("\nSafely Terminating Server...\n");

	server_finished = true;

	exit(signum);

}
//Definition
bool create_image(data_link_layer * d, std::string file);
//Author: Stephen Michelini
int main(void) {
	server_finished = false;
	//Register Ctrl-C Kill to Safely Clean-Up Server
	//Adapted from http://www.yolinux.com/TUTORIALS/C++Signals.html
	signal(SIGINT, signal_callback_handler);


	s = new tcp_server(LISTEN_PORT);
	if(!s->start()){
		std::cout<<"Failed to Start Server"<<std::endl;
		delete s;
		return 1;
	}
	//Let the user know we have started
	std::cout<<"Concurrent Photo Server Started!"<<std::endl;
	//Keep track of how many clients we have
	unsigned int clientId = 0;
	//Run in this til our signal interrupts us
	while(!server_finished){
		//Get a socket
		long fd = s->acceptSocket();
		//Error on accept() failure
		if(fd <= 0){
			std::cout<<"Failed to Accept Socket"<<std::endl;
			delete s;
			return 1;
		}
		//New client
		clientId++;
		//Fork the process
		switch(fork()){
		//fork() error
		case -1:
			//This is a severe error and we will not attempt to recover
			std::cout<<"Error: Making New Process"<<std::endl;
			server_finished = true;
			delete s;
			exit(0);
		//Child Process
		case 0:{
			//Kill the server
			delete s;
			//Buffers for clientId string and photo string
			char clientIdBuf[4];
			char imageIdBuf[4];
			//Create the client ID
			sprintf(clientIdBuf, "%d", clientId);
			//Log file name will be server_(client id).log
			std::string filename = "server_" + std::string(clientIdBuf) + ".log";
			//Data Link Layer Object
			data_link_layer * d;
			d = new data_link_layer(fd,filename);
			unsigned int photoId = 1;
			while(1){
				sprintf(imageIdBuf, "%d", photoId);
				//keep creating images from client until they close the connection
				if(!create_image(d, "photonew"+std::string(clientIdBuf)+std::string(imageIdBuf)+".jpg"))
				{
					break;
				}
				photoId++;
			}

			delete d;
			exit(0);
		}
		//Parent Process
		default:{
			//Close the file descriptor
			close(fd);
			continue;
		}
		}
	}

	delete s;
	return EXIT_SUCCESS;
}
/*!
 * Create's an image from the packets recieved from the network
 *
 * @author msbeaulieu
 */
bool create_image(data_link_layer * d, std::string file){
	//Reading Buffer
	char buf[READ_SIZE];
	//Clear buffer
	memset(buf, 0, READ_SIZE);
	//If we can't read any data no sense in making the file
	long rc = d->readData(buf, READ_SIZE);
	if(rc <= 0){
		return false;
	}
	std::ofstream f(file.c_str(), std::ios::trunc| std::ios::out| std::ios::binary);

	//Write the first bytes
	f.write(buf, rc);
	//Ack the packet
	d->sendPacketAck();
	//It's not over til its over
	bool done = false;
	while(!done&&!server_finished){
		//Clear the buffer
		memset(buf, 0, READ_SIZE);
		//Read in Some Data
		rc = d->readData(buf, READ_SIZE);
		//Error out
		if(rc <= 0){
			std::cout<<"Error Reading Image"<<std::endl;
			return false;
		}
		//Write out
		f.write(buf, rc);
		//Ack
		d->sendPacketAck();
		//JPEG final image part
		//All JPEGs end in 0xffd9
		if((buf[rc-2]==(char)0xFF)&&(buf[rc-1]==(char)0xd9)){
			break;
		}
	}
	//Keep tidy and close the file
	f.close();
	return true;
}
