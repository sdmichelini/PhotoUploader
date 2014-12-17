/*
 * tcp_client.h
 *
 *  Created on: Nov 4, 2014
 *      Author: sdmichelini
 */

#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <iostream>
//Has memcpy
#include <cstring>
///Unix Includes from P and D book
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>

///The size of a read buffer for TCP
#define MAX_BUFFER_SIZE 255

//Uncomment to show debug from tcp_client
//#define TCP_CLIENT_DEBUG

//Uncomment this to disable interacting with a server to see which bytes would have been sent out
//#define NO_SERVER

///Using Class from prog0
/*!
 * @brief TCP Client Implementation
 *
 * This class manages a TCP client from initializing it to connecting to a remote server.
 * Handles all BSD API. Serve's as a model for all TCP clients
 */
class tcp_client{
protected:
	/*!
	 * Initializes the Socket. Does no network operations
	 */
	tcp_client();
	/*!
	 * Connects to a remote server.
	 * Call this first before doing any other socket operations.
	 * @param remoteIP
	 * 	Remote IP or Host Name of the Server
	 * @param port
	 * 	Port to communicate on
	 * @return
	 * 	true on successful connection, false on failed connection
	 */
	bool connectToServer(char * remoteIP, unsigned short port);
	/*!
	 * Writes to a remote connection
	 * @param message
	 * 	Message to write to server
	 * @param size
	 * 	How many bytes the message is
	 * @return
	 * 	number of bytes on successful write
	 */
	long writeToSocket(char * message, unsigned long size);
	/*!
	 * Reads from a remote connection
	 * @param array
	 * 	Pointer to an allocated array that is passed and filled with the return string
	 * @param maxSize
	 * 	Max Size of the array to be allocated. Can not be bigger than MAX_BUFFER_SIZE
	 * @return
	 * 	size of the buffer read, 0 on error or no read
	 */
	long readFromSocket(char * array, unsigned long maxSize);
	/*!
	 * Reads from a remote connection using the select method
	 * @param array
	 * 	Pointer to an allocated array that is passed and filled with the return string
	 * @param maxSize
	 * 	Max Size of the array to be allocated. Can not be bigger than MAX_BUFFER_SIZE
	 * 	@param timeout
	 * 	Timeout in milliseconds
	 * @return
	 * 	size of the buffer read, 0 on error or no read
	 */
	long readFromSocketWithTimeout(char * array, unsigned long maxSize, unsigned long timeout);
	/*!
	 * Terminates the TCP connection
	 */
	void closeSocket();
	/*!
	 * Checks whether or not the socket is connected
	 * @return
	 * 	true if connected, false is not connected
	 */
	bool isConnected()
	{
		return bConnected;
	}
	///Destructor
	~tcp_client();
protected:
	///Socket File Descriptor
	long lSocket;
	///Is the socket connected
	bool bConnected;
};
//Typedef to more easily abstract
typedef tcp_client physical_layer;


#endif /* TCP_CLIENT_H_ */
