/*
 * data_link_layer.h
 *
 *  Created on: Dec 11, 2014
 *      Author: sdmichelini
 */

#ifndef DATA_LINK_LAYER_H_
#define DATA_LINK_LAYER_H_

#include "tcp_client.h"

//For logging operations
#include <fstream>
///End of Packet Byte
#define END_OF_PACKET_BYTE 0x01
#define NOT_END_OF_PACKET 0x00
///Maximum Buffer Size
#define BUFFER_SIZE 255
///Payload Size
#define PAYLOAD_SIZE 124
///Ack frame size
#define ACK_FRAME_SIZE 4
///Ack packet size
#define ACK_PACKET_SIZE 5
///Ack timeout in milliseconds
#define ACK_TIMEOUT 500//no exponential backoff. maybe implement for further projects?
///Artificial Error Injection
#define ARTIFICIAL_ERROR


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



/*!
 * Data-Link Layer Abstraction point
 *
 * This class utilizes a local buffer which can take larger packet and convert them into frames
 *
 * It implements the PAR protocol
 */
class data_link_layer: private physical_layer{
public:
	data_link_layer(std::string logFileName);
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
	bool connectToRemoteHost(char * remoteIP, unsigned short port);
	/*!
	 * Send's data to the physical layer
	 */
	bool sendData(char * data, unsigned long size);
	///Wait for Packet Ack
	bool waitForPacketAck();
	~data_link_layer();
private:
	///Flushes the Send Buffer
	void flushSendBuffer();
	///Wait's for an acknowledgment from server.
	bool waitForSendAck();

	///Calculate the XOR folding check
	uint16_t calculateXorCheck(char * packet, unsigned long packetSize);

	//Variables
	///Buffer for data sent to the physical layer
	uint8_t m_sendBuf[BUFFER_SIZE];
	///Send Buffer offset
	uint8_t m_sendBufOffset;
	///Packet Sequence Number
	uint16_t m_sequenceNumber;
	///Packet Count Number
	uint16_t m_packetCount;

	///Logfile
	std::ofstream m_logFile;

//Only use this if artificial error is being injected
#ifdef ARTIFICIAL_ERROR
	unsigned long transmissionTries;
#endif
};


#endif /* DATA_LINK_LAYER_H_ */
