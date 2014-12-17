/*
 * tcp_socket.cpp
 *
 *  Created on: Dec 8, 2014
 *      Author: sdmichelini
 */
#include "tcp_socket.h"

///@author msbeaulieu
tcp_socket::tcp_socket(long socket_fd){
	//Init Variables
	m_socket_fd = socket_fd;
	m_connected = true;

	//Set up file descriptor sets
	FD_ZERO(&m_activefds);
	FD_SET(socket_fd, &m_activefds);
}

///@author msbeaulieu
bool tcp_socket::writeToSocket(char * message, unsigned long size)
{
#ifdef NO_CLIENT
	std::cout<<"Writing to Socket: "<<message;
	return true;
#endif

	//Don't send anything if the size is zero or we are not connected
	if(size==0||!m_connected)
	{

		return false;
	}
	//Will return the bytes sent over the network
	size_t bytesSent = send(m_socket_fd, message, size, 0);
	//Check we sent the bytes that were intended to be sent
	if(bytesSent!=size)
	{
		std::cout<<"Error: Writing to Socket"<<std::endl;
		return false;
	}
	return true;

}

///@author msbeaulieu
long tcp_socket::readFromSocket(char * array, unsigned long maxSize)
{
#ifdef NO_CLIENT
	return 0;
#endif

	//Don't read unless we are connected, listening and intend to read one or more bytes
	if(maxSize==0||!m_connected)
	{
		return false;
	}
	//Here we read up to maxSize
	//We use the max size because if a user allocated the buffer they can specify how big it is
	if(maxSize > MAX_BUFFER_SIZE)
	{
		maxSize = MAX_BUFFER_SIZE;
	}
	long rc = recv(m_socket_fd, array, maxSize, 0);
	//Socket Error
	if(rc < 0)
	{
		std::cout<<"Socket Error on Read"<<std::endl;
	}
	//Read nothing
	else if(rc == 0)
	{
		std::cout<<"No Data Received from Socket"<<std::endl;
	}
	return rc;
}

///@author msbeaulieu
long tcp_socket::readFromSocketWithTimeout(char * array, unsigned long maxSize, unsigned long millis){
#ifdef NO_CLIENT
	return 0;
#endif
	//Don't read unless we are connected, listening and intend to read one or more bytes
	if(maxSize==0||!m_connected)
	{
		return 0;
	}
	//FD Set
	fd_set readFds;
	FD_ZERO(&readFds);
	FD_SET(m_socket_fd,&readFds);

	//Timeout
	timeval t;
	//Seconds portion
	t.tv_sec = millis/1000;
	//Microseconds portion
	t.tv_usec = (millis%1000) * 1000;
	//Limit to MAX_BUFFER_SIZE
	if(maxSize > MAX_BUFFER_SIZE)
	{
		maxSize = MAX_BUFFER_SIZE;
	}
	long count;
	if(select(m_socket_fd + 1, &readFds, NULL, NULL, &t) > 0){
		if(FD_ISSET(m_socket_fd, &readFds)){
			//Read from Socket
			count = recv(m_socket_fd, (char *)array, maxSize, 0);

		}//fd_isset
		else{
			std::cout<<"Select Timeout of: "<<t.tv_sec<<" seconds and "<<t.tv_usec<<" milliseconds has passed"<<std::endl;
			count = 0;
		}
	}//select
	else{
		std::cout<<"select() error"<<std::endl;
		count = -1;
	}
	return count;
}

///@author msbeaulieu
void tcp_socket::closeSocket(){
	if(m_connected){
		m_connected = false;
		close(m_socket_fd);
	}
}

///@author msbeaulieu
tcp_socket::~tcp_socket(){
	//Clean-Up
	closeSocket();
}
