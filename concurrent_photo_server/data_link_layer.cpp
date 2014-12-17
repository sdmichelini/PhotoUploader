/*
 * data_link_layer.cpp
 *
 *  Created on: Dec 8, 2014
 *      Author: sdmichelini
 */
#include "data_link_layer.h"

///@author msbeaulieu
data_link_layer::data_link_layer(long socketFd, std::string logFile):tcp_socket(socketFd),m_logFile(logFile.c_str(), std::ios::out | std::ios::trunc){
	//Client starts off at 1
	m_remoteSequence = 1;
	m_remotePacket = 1;
	m_remoteFrame = 1;
#ifdef ARTIFICIAL_ERROR
	transmissionTries = 0;
#endif
}

///@author sdmichelini
long data_link_layer::readData(char * buf, unsigned long size){
	//Local Buffer
	char recvBuf[RECV_BUFFER_SIZE];
	//Cap receive buffer size at RECV_BUFFER_SIZE
	if(size > RECV_BUFFER_SIZE){
		size = RECV_BUFFER_SIZE;
	}
	//This will be split up into receiving frames
	//Total Frame Bytes Received
	unsigned long recvBytes = 0;
	//Get all the data
	while(recvBytes < size){
		//Clear the local buf
		memset(recvBuf, 0, RECV_BUFFER_SIZE);
		//Read in the frame

		unsigned long rc = readFrame(recvBuf, size);
		if(rc == 0){
			//std::cout<<"Error: Data Link Layer - No Frame Received"<<std::endl;
			return 0;
		}
		else{
			//Check end of packet byte
			if(recvBuf[rc - 1] == END_OF_PACKET_BYTE){

			}
			//Move data to buf
			memcpy(buf + recvBytes, recvBuf, rc - 1);
			recvBytes += (rc -1);
			//Check end of packet byte
			if(recvBuf[rc - 1] == END_OF_PACKET_BYTE){
				m_logFile<<"Packet "<<m_remotePacket<<" sent to network layer"<<std::endl;
				m_remoteFrame = 1;
				//Increment Packet count
				m_remotePacket++;

				return recvBytes;

			}
		}
	}
	//This case is if the full buffer is read
	m_remoteFrame = 1;
	m_remotePacket++;
	return recvBytes;
}
///@author sdmichelini
void data_link_layer::sendPacketAck(){
	//ACK Byte
	//Network Endian Remote Sequence
	uint16_t nRemoteSequence = htons(m_remoteSequence - 1);
	char buf[5];
	memcpy(buf, &nRemoteSequence, 2);

	//ACK packet
	buf[2] = 0x06;
	uint16_t errorBytes = xorFold(buf, 3);
	memcpy(buf + 3, &errorBytes, sizeof(uint16_t));
	this->writeToSocket(buf, 5);

	m_logFile<<"Ack Packet: "<<m_remotePacket<<" sent."<<std::endl;
}

///@author sdmichelini
long data_link_layer::readFrame(char * buf, unsigned long size){
	//Local Buffer
	char localBuf[RECV_BUFFER_SIZE];


	//Assume bad frame
	bool goodFrame = false;
	//Loop til we get one
	while(!goodFrame){
		//Clear Local Buffer
		memset(localBuf, 0, RECV_BUFFER_SIZE);
		//Read from the physical layer
		long rc = readFromSocketWithTimeout(localBuf, size, READ_TIMEOUT);
		if(rc <= 0){
			//std::cout<<"Data Link Layer: Error Frame Read Timeout"<<std::endl;
			return 0;
		}
		//Now we extract the data
		//The frame will be size rc
		//So we know the data size will be (rc - seqSize - xorFoldSize)
		//Error if that is not the case
		if((int)(rc - (int)sizeof(uint16_t) - (int)sizeof(uint16_t)) < 0){
			std::cout<<"Error: Data Link Layer - Frame is less than Required Size"<<std::endl;
			return 0;
		}
		//remote sequence number
		uint16_t frameSeq;
		//remote error number
		uint16_t errorCheck;

		memcpy(&frameSeq, localBuf, sizeof(uint16_t));
		//Convert endian
		frameSeq = ntohs(frameSeq);
		//Check we are receiving proper frame
		if(frameSeq != m_remoteSequence){
			std::cout<<"Bad Sequence of: "<<frameSeq<<" expected: "<<m_remoteSequence<<std::endl;
			//Ack it anyway
			//Network Endian Remote Sequence
			uint16_t nRemoteSequence = htons(frameSeq);
			char buf[4];
			memcpy(buf, &nRemoteSequence, 2);
			memcpy(buf + 2, &nRemoteSequence, 2);
			this->writeToSocket(buf, 4);
			m_logFile<<"Ack Frame Received in Error: "<<m_remoteFrame<<" of Packet: "<<m_remotePacket<<". Sent an ack."<<std::endl;
			//Continue the loop
			continue;
		}
		//Check XOR fold
		//Grab the last two bytes
		memcpy(&errorCheck, localBuf+(rc - sizeof(uint16_t)), sizeof(uint16_t));
		uint16_t packetError = xorFold(localBuf, rc - sizeof(uint16_t));
		//Check XOR Fold
		if(errorCheck != packetError){
			m_logFile<<"Frame "<<m_remoteFrame<<" of packet "<<m_remotePacket<<" damaged."<<std::endl;
			//Continue the loop
			continue;
		}
		m_logFile<<"Frame "<<m_remoteFrame<<" of packet "<<m_remotePacket<<" received"<<std::endl;
		//Copy the frame data
		memcpy(buf, localBuf+sizeof(uint16_t),(rc - sizeof(uint16_t) - sizeof(uint16_t)));
		//Now we have a good ack
		//Ack the frame
		sendFrameAck();
		return (rc - sizeof(uint16_t) - sizeof(uint16_t));
	}
	return size;
}

///@author msbeaulieu
void data_link_layer::sendFrameAck(){
	//Network Endian Remote Sequence
	uint16_t nRemoteSequence = htons(m_remoteSequence);
	char buf[4];
	memcpy(buf, &nRemoteSequence, 2);
#ifdef ARTIFICIAL_ERROR
	transmissionTries++;
	uint16_t error = ((transmissionTries % 13)==12)?~nRemoteSequence:nRemoteSequence;
	memcpy(buf + 2, &error, 2);
#else
	memcpy(buf + 2, &nRemoteSequence, 2);
#endif
	this->writeToSocket(buf, 4);
	m_logFile<<"Ack Frame: "<<m_remoteFrame<<" of Packet: "<<m_remotePacket<<" sent."<<std::endl;
	m_remoteFrame++;
	m_remoteSequence++;
}
///@author sdmichelini
uint16_t data_link_layer::xorFold(char * array, unsigned long size){
	uint16_t result = 0;
	for(unsigned int i = 0; i < (size/2); i++){
		uint16_t curNum = 0;

		//Copy in the current packet
		memcpy(&curNum, (char *)(array + 2*i), sizeof(uint16_t));

		//XOR fold
		result ^= curNum;
	}
	return result;
}
///@author msbeaulieu
data_link_layer::~data_link_layer(){
	//Clean-Up
	closeSocket();
	m_logFile.close();
}



