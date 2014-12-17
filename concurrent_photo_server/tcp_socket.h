/*
 * tcp_socket.h
 *
 *  Created on: Dec 8, 2014
 *      Author: sdmichelini
 */

#ifndef TCP_SOCKET_H_
#define TCP_SOCKET_H_


#include <sys/time.h>
#include <iostream>
//Has memcpy
#include <cstring>
///Unix Includes from P and D book
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

///The size of a read buffer for TCP
#define MAX_BUFFER_SIZE 256



/*!
 * Class to manage a TCP connection
 *
 * This class will function as the physical layer in the network
 */
class tcp_socket{
public:
	/*!
	 * Constructs the TCP socket with a file descriptor
	 * This comes from the accept or connect calls
	 */
	tcp_socket(long socket_fd);
	/*!
	 * Terminates the connection to a remote client
	 */
	void closeSocket();
	/*!
	 * Writes to a remote connection
	 * @param message
	 * 	Message to write to server
	 * @param size
	 * 	How many bytes the message is
	 * @return
	 * 	true on successful write, false on failed write
	 */
	bool writeToSocket(char * message, unsigned long size);

	/*!
	 * Reads from a remote connection
	 * @param array
	 * 	Pointer to an allocated array that is passed and filled with the return string
	 * @param maxSize
	 * 	Max Size of the array allocated. Can not be bigger than MAX_BUFFER_SIZE
	 * @return
	 * 	size of the buffer read, 0 on error or no read
	 */
	long readFromSocket(char * array, unsigned long maxSize);

	/*!
	 * Reads from a remote connection with a timeout
	 * Uses the select system call along with creating a timeval
	 * * @param array
	 * 	Pointer to an allocated array that is passed and filled with the return string
	 * @param maxSize
	 * 	Max Size of the array allocated. Can not be bigger than MAX_BUFFER_SIZE
	 * @return
	 * 	size of the buffer read, 0 on error or no read
	 */
	long readFromSocketWithTimeout(char * array, unsigned long maxSize, unsigned long millis);

	/*!
	 * Get the native file descriptor from the class
	 */
	long getNativeHandle(){
		return m_socket_fd;
	}

	/*!
	 * Return's whether or not the remote socket is connected
	 */
	bool isConnected(){
		return m_connected;
	}

	///Destructor
	virtual ~tcp_socket();
private:
	///file descriptor set
	fd_set m_activefds;
	///file descriptors to read from
	fd_set m_readfds;
	///Socket file descriptor
	long m_socket_fd;
	///Whether or not the socket is connected
	bool m_connected;
};

/*!Typedef for physical_layer
 * make's it easier to think abstratly about the network
 */
typedef tcp_socket physical_layer;


#endif /* TCP_SOCKET_H_ */
