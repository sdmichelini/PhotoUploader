/*
 * tcp_server.cpp
 *
 *  Created on: Nov 8, 2014
 *      Author: sdmichelini
 */
#include "tcp_server.h"

///@author sdmichelini
tcp_server::tcp_server(unsigned short port)
{
	//Init all class variables
	nPort = port;
	lServerSocket = 0;
	bListening = false;
}

///@author sdmichelini
bool tcp_server::start()
{
#ifdef NO_CLIENT
	std::cout<<"Warning: Disable NO_CLIENT to start real server"<<std::endl;
	return true;
#endif

	struct sockaddr_in sSockInfo;//Socket Information Structure
	char myHostname[MAX_HOSTNAME];//Holder for HOSTNAME
	struct hostent * pHostName;//Pointer to our hostent

	memset(&sSockInfo, 0, sizeof(sockaddr_in));//Clear the memory of sSockInfo


	gethostname(myHostname,MAX_HOSTNAME-1);//Copy the system host name into myHostname

	if((pHostName=gethostbyname(myHostname))==NULL)//Check for Error in Getting Our Host
	{
		std::cout<<"Host Name Error on Server with Host Name: "<<myHostname<<std::endl;
		return false;
	}

	sSockInfo.sin_family = AF_INET;//IPv4

	sSockInfo.sin_addr.s_addr = htonl(INADDR_ANY);//Use any available address for the Server

	sSockInfo.sin_port = htons(nPort);//Port


	if((lServerSocket = socket(AF_INET,SOCK_STREAM,0))<0)//Create the socket
	{
		std::cout<<"Failure Creating the Socket"<<std::endl;
		return false;
	}


	if(bind(lServerSocket,(struct sockaddr *)&sSockInfo,sizeof(sockaddr_in)) < 0)//Now bind the socket
	{
		std::cout<<"Failure Binding Socket"<<std::endl;
		std::cout<<"Error: "<<strerror(errno)<<std::endl;
		return false;
	}

	listen(lServerSocket,LISTEN_BACKLOG);//Listen
	bListening = true;//Safeguard for Writing and Reading from Socket
	return true;
}

///@author sdmichelini
long tcp_server::acceptSocket()
{
#ifdef NO_CLIENT
	std::cout<<"Warning: Disable NO_CLIENT to utilize real network calls"<<std::endl;
	return 0;
#endif
	long lSocket = 0;
	//Accept the socket or return an error
	if((lSocket = accept(lServerSocket,NULL,NULL)) < 0)
	{
		std::cout<<"Failure Accepting Socket"<<std::endl;
		return 0;
	}
	return lSocket;
}

///@author sdmichelini
void tcp_server::closeServer()
{
	//Only close socket once
	if(bListening)
	{
		bListening = false;
		close(lServerSocket);
	}
}

///@author sdmichelini
tcp_server::~tcp_server()
{
	//Clean Up
	closeServer();
}
