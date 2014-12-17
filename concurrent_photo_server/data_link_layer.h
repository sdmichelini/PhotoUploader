/*
 * data_link_layer.h
 *
 *  Created on: Dec 8, 2014
 *      Author: sdmichelini
 */

#ifndef DATA_LINK_LAYER_H_
#define DATA_LINK_LAYER_H_

#include "tcp_socket.h"
#include <string>
#include <fstream>

///Number of seconds to wait for receiving data from the remote client
#define READ_TIMEOUT 10000
///Local receive buffer size
#define RECV_BUFFER_SIZE 255
///End of Packet Byte
#define END_OF_PACKET_BYTE 0x01
#define NOT_END_OF_PACKET 0x00

///Data Frame Struct
struct data_frame{
	///Sequence Number
	uint16_t sequenceNumber;
	///Pay Load
	char * payload;
	///End of Packet Byte
	uint8_t endOfPacket;
	///Error Detection Bytes
	uint16_t errorDetection;
};

///Artificial Error Injection
#define ARTIFICIAL_ERROR

/*!
 * Emulated Data-Link Layer
 * A positive acknowledgment and retransmission protocol implementation
 *
 * The physical_layer is not exposed when this class is instantiated because all network calls from the application layer must
 * pass through the data link layer to get to the physical layer
 */
class data_link_layer: private physical_layer{
public:
	data_link_layer(long socketFd, std::string logfile);
	/*!
	 * Read's data from a remote client
	 * will reassemble the frame of packets
	 *
	 * @param buf
	 * 	Buffer to place the packet in
	 * @param size
	 * 	Size of Buffer to be Read
	 * @return
	 * 	Size of Packet Read
	 */
	long readData(char * buf, unsigned long size);
	/*!
	 * Send's a received packet ack
	 */
	void sendPacketAck();
	///Desctructor
	~data_link_layer();
private:
	/*!
	 *	Read's a frame the remote server and will ack it
	 */
	long readFrame(char * buf, unsigned long size);
	///Send an ack packet back to remote client
	void sendFrameAck();
	///XOR fold
	uint16_t xorFold(char * array, unsigned long size);
	///Remote Sequence Number
	unsigned long m_remoteSequence;
	///Remote Packet Number
	unsigned long m_remotePacket;
	///Remote Frame Number
	unsigned long m_remoteFrame;
	///Log File Name
	std::ofstream m_logFile;
//only keep track of how many times we tried to transmit if there is artificial error being injected
#ifdef ARTIFICIAL_ERROR
	unsigned long transmissionTries;
#endif
};


#endif /* DATA_LINK_LAYER_H_ */
