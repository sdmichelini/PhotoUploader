/*
 * tcp_server.h
 *
 *  Created on: Nov 8, 2014
 *      Author: sdmichelini
 */

#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <iostream>
//Has memcpy
#include <cstring>
///Unix Includes from P and D book
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

///The size of a read buffer for TCP
#define MAX_BUFFER_SIZE 256
///The max size of the hostname we are running on
#define MAX_HOSTNAME 255
///Server Port
#define SERVER_PORT 36500
///Backlog for listen(number of clients)
#define LISTEN_BACKLOG 2

///Allows no client to be used to test applications not as a server
//#define NO_CLIENT //Uncomment to Not use network

/*!
 * @brief Manages a TCP Server
 *
 * This implementation of a TCP server allows for easy communication to a single client.
 * Note: you must call acceptSocket and then do your writing and reading operations and when you are done
 * with the client call closeSocket.
 * This can be easily abstracted to implement TCP protocols such as HTTP and FTP
 */
class tcp_server{
public:
	/*!
	 * Create's a TCP Server on a specified port.
	 * Note: Does not start the TCP server. Call start to do that
	 * @param port
	 * 	Port to have the Server on. Setting it to zero will allow OS to choose a port.
	 */
	tcp_server(unsigned short port);
	/*!
	 * Start's the TCP Server. Does the binding of the socket
	 * @return
	 * 	true on successful startup, false on failure
	 */
	bool start();
	/*!
	 * Tries to accept a remote client
	 * @return
	 * 	socket_fd from accepted connection
	 */
	long acceptSocket();

	/*!
	 * Stop the server closes it
	 */
	void closeServer();

	/*!
	 * Checks whether or not we are listening for clients
	 */
	bool isListening()
	{
		return bListening;
	}
	/*!
	 * Returns the port that the server is configured to listen on
	 */
	unsigned short getPort()
	{
		return nPort;
	}
	/*!
	 * Cleans up the server
	 */
	virtual ~tcp_server();
private:
	///Port which the server binds to
	unsigned short nPort;
	///Socket FD for server
	long lServerSocket;
	///Whether or not the server is listening
	bool bListening;
};

#endif /* TCP_SERVER_H_ */
