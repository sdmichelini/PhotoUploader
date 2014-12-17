/*
 * tcp_client.cpp
 *	Implementation of a TCP Client
 *  Created on: Nov 4, 2014
 *      Author: sdmichelini
 */
#include "tcp_client.h"
#include <fstream>
///@author msbeaulieu
tcp_client::tcp_client()
{
	///Init all variables
	///Socket is zero because we have none
	lSocket = 0;
	///We are not connected
	bConnected = false;
}
///@author sdmichelini
bool tcp_client::connectToServer(char * remoteIP, unsigned short port)
{
#ifdef NO_SERVER
	std::cout<<"NO_SERVER enabled."<<std::endl;
	return true;
#endif
	//Get the IP of the remote server
	//Using example from P and D
	//Pointer to our hostnet
	struct hostent * pHostnet;
	//Remote Socket Info
	struct sockaddr_in sSocketInfo;
	//Resolve the host name
	pHostnet = gethostbyname(remoteIP);
	//Error if we fail to find an IP for host name
	if(!pHostnet)
	{
		std::cout<<"Error: Could not resolve host name: "<<remoteIP<<std::endl;
		return false;
	}
	//Zero out the socket memory
	//Not using bzero here because it is unsupported in some BSD implementations such as Cygwin
	memset(&sSocketInfo,0,sizeof(sSocketInfo));

	//Now copy over socket information
	memcpy((char*)&sSocketInfo.sin_addr, pHostnet->h_addr, pHostnet->h_length);
	//IPv4
	sSocketInfo.sin_family = AF_INET;
	//Convert from little endian to big endian for network
	sSocketInfo.sin_port = htons(port);
	//Create the socket
	if((lSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		std::cout<<"Error: Could not create socket."<<std::endl;
		return false;
	}

	//Connect to the remote host
	if(connect(lSocket,(struct sockaddr *)&sSocketInfo,sizeof(sSocketInfo)) < 0)
	{

		//std::cout<<"Error: Could not connect to IP:"<<remoteIP<<" on port: "<<port<<std::endl;
		return false;
	}
	//If we get here It must be connected
	bConnected = true;
	return true;
}
///@author sdmichelini
void tcp_client::closeSocket()
{
#ifdef NO_SERVER
	std::cout<<"NO_SERVER enabled."<<std::endl;
	return;
#endif
	//Don't call close twice
	if(bConnected)
	{
		bConnected = false;
		close(lSocket);
		lSocket = 0;
	}
}
///@author sdmichelini
long tcp_client::writeToSocket(char * message, unsigned long size)
{

#ifdef NO_SERVER
	printf("Hex Message: ");
	for(unsigned int i = 0; i < size; i++){
		printf("%02x",message[i]);
	}
	printf("\n");
	return size;
#endif
	//Don't send anything if the size is zero or we are not connected
	if(size==0||!bConnected)
	{
		std::cout<<"Error: Message size is 0 or socket not connected"<<std::endl;
		return -1;
	}
	//Will return the bytes sent over the network
	size_t bytesSent = send(lSocket, message, size, 0);
	//Check we sent proper amount of bytes
	if(bytesSent!=size)
	{
#ifdef TCP_CLIENT_DEBUG
		std::cout<<"Error: Writing to Socket"<<std::endl;
#endif
		return -1;
	}
	return bytesSent;

}
///@author sdmichelini
long tcp_client::readFromSocket(char * array, unsigned long maxSize)
{
	//Don't perform a read if we are not connected or we are requesting a message of 0 bytes
	if(maxSize==0||!bConnected)
	{
		return false;
	}
	//Limit to MAX_BUFFER_SIZE
	if(maxSize > MAX_BUFFER_SIZE)
	{
		maxSize = MAX_BUFFER_SIZE;
	}
	//Read from Socket
	long rc = recv(lSocket, array, maxSize, 0);

	//Error on socket
	if(rc < 0)
	{
		std::cout<<"Socket Error on Read"<<std::endl;
	}
	//No data received
	else if(rc == 0)
	{
#ifdef TCP_CLIENT_DEBUG
		std::cout<<"No Data Received from Socket"<<std::endl;
#endif
	}
	return rc;
}
///@author sdmichelini
long tcp_client::readFromSocketWithTimeout(char * array, unsigned long maxSize, unsigned long timeout){
	//Don't perform a read if we are not connected or we are requesting a message of 0 bytes
	if(maxSize==0||!bConnected)
	{
		return false;
	}
	//FD Set
	fd_set readFds;
	FD_ZERO(&readFds);
	FD_SET(lSocket,&readFds);

	//Timeout
	timeval t;
	//Seconds portion
	t.tv_sec = timeout/1000;
	//Microseconds portion
	t.tv_usec = (timeout%1000) * 1000;
	//Limit to MAX_BUFFER_SIZE
	if(maxSize > MAX_BUFFER_SIZE)
	{
		maxSize = MAX_BUFFER_SIZE;
	}
	//Bytes read
	long count;
	//Try to use select
	if(select(lSocket + 1, &readFds, NULL, NULL, &t) > 0){
		//Check if FD is set
		if(FD_ISSET(lSocket, &readFds)){
			//Read from Socket
			count = recv(lSocket, array, maxSize, 0);

		}//fd_isset
		else{
			count = 0;
		}
	}//select
	else{
		//Error should be handled by network layer
		//std::cout<<"select() error"<<std::endl;
		count = -1;
	}

	return count;
}
///@author msbeaulieu
tcp_client::~tcp_client()
{
	//Clean-Up
	closeSocket();
}



