//============================================================================
// Name        : concurrent_photo_client.cpp
// Author      : Stephen Michelini and Matthew Beaulieu
// Version     :
// Copyright   : 
// Description : Concurrent Photo Client
//============================================================================

#include "data_link_layer.h"

#include <stdio.h>
#include <stdlib.h>

#include <fstream>

#define PHOTO_BUF_SIZE 200

//For local testing purposes only
//#define REMOTE_HOST "127.0.0.1"

#define SERVER_PORT 36500

///Remote Host to Connect To
char * remoteHost;
//Definition
void upload_photo(data_link_layer *d, char * clientId, unsigned int photoId);
//Author: sdmichelini
/*!
 * Serves as the application/network layer for program
 * @author msbeaulieu
 */
int main(int argc, char * argv[]) {
	//Argc must be 4
	if(argc != 4){
		std::cout<<"Error: Usage ./concurrent_photo_client (server) (client_id(integer)) (num_photos(integer)"<<std::endl;
	}
	//Use the same connection for all photos
	data_link_layer *d;
	//Remote host
	remoteHost = argv[1];

	d = new data_link_layer("client_"+std::string(argv[2])=".log");
	//Error if the num_photos is less than or equal to zero
	if(atoi(argv[2]) <= 0){
		std::cout<<"Concurrent Photo Client will not upload if you don't have any photos.";
		delete d;
		return 1;
	}
	//Try to upload each photo
	for(unsigned int i = 1; i <= atoi(argv[3]); i++){
		upload_photo(d, argv[2], i);
	}
	delete d;
	return EXIT_SUCCESS;
}
/*upload_photo
 * @brief Uploads a Photo to the Photo Server
 * @author: msbeaulieu
 */
void upload_photo(data_link_layer *d, char* clientId, unsigned int photoId){
	//Connect only once if we have a photo that we can upload
	static bool connected = false;
	//For int to C-String conversion
	char numBuf[4];
	//Do the actual conversion
	sprintf(numBuf, "%d", photoId);
	//Filename
	std::string filename = "photo" + std::string(clientId) + std::string(numBuf) + ".jpg";
	//File object to read from
	std::ifstream file(filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
	unsigned long size;

	//File opened successfully
	if(file.is_open()){
		//Where is the end of file
		size = file.tellg();
		//Move to beginning of file
		unsigned long rc = 0;
		file.seekg(0, std::ios::beg);
		char photoBuf[PHOTO_BUF_SIZE];
		//Keep reading until the EOF
		while(rc < size){
			//Read in at max chunks of 200
			unsigned long readSize;
			readSize = ((size - rc) > PHOTO_BUF_SIZE)?PHOTO_BUF_SIZE:(size-rc);
			//Clear the buffer
			memset(photoBuf, 0, PHOTO_BUF_SIZE);
			//Read
			file.read(photoBuf, readSize);
			//If we are not connected attempt to connect
			if(!connected){
				connected = d->connectToRemoteHost(remoteHost,SERVER_PORT);
				//Error out if not connected
				if(!connected){
					std::cout<<"We could not connect to the upload server at this time."<<std::endl;
					break;
				}
			}
			//Send the packet
			d->sendData(photoBuf,readSize);
			//Wait for the packet to be acked
			d->waitForPacketAck();
			//We have read in the data
			rc += readSize;
		}
		//Only say we uploaded photo when we are connected
		if(connected){
			std::cout<<"Upload "<<filename<<" has finished uploading to the Server!"<<std::endl;
		}

		//Close file when done
		file.close();
	}
	else{
		std::cout<<"Failed to Upload File: "<<filename<<". Is it spelled correctly?"<<std::endl;
	}
}
