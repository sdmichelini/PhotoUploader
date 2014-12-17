/*
 * data_link_layer.cpp
 *
 *  Created on: Dec 14, 2014
 *      Author: sdmichelini
 */
#include "data_link_layer.h"

///@author msbeaulieu
data_link_layer::data_link_layer(std::string logFileName):m_logFile(logFileName.c_str(), std::ofstream::trunc){
	//Clear the memory of the buffer
	memset(m_sendBuf, 0, BUFFER_SIZE);
	//Original Buffer Offset is Zero
	m_sendBufOffset = 0;
	//First packet send will be sequence number 0
	m_sequenceNumber = 1;
	m_packetCount = 1;
#ifdef ARTIFICIAL_ERROR
	transmissionTries = 0;
#endif

}
///@author msbeaulieu
bool data_link_layer::connectToRemoteHost(char * remoteIP,unsigned short port){
	//Calls to the physical layer
	return connectToServer(remoteIP, port);
}

///@author sdmichelini
bool data_link_layer::sendData(char * data, unsigned long size){
	if(size > BUFFER_SIZE){
		std::cout<<"Error: Data Link Layer: Data Size of: "<<size<<" is too big to fit in the send buffer."<<std::endl;
		return false;
	}
	//Check that there is room in our send buffer or else flush it
	else if(size + m_sendBufOffset > BUFFER_SIZE){
		flushSendBuffer();
	}
	memset(m_sendBuf, 0, BUFFER_SIZE);
	//Now load the memory into the buffer
	memcpy(m_sendBuf + m_sendBufOffset, data, size);
	//Sent bytes counter
	unsigned long bytesSent = 0;
	unsigned long frame = 1;

	//Send in chunks
	while(bytesSent != size){

		data_frame d;
		//Bytes sent delta
		unsigned long bytesSentDelta = 0;
		bool goodAck = false;
		while(!goodAck)
		{
			char buf[BUFFER_SIZE];
			unsigned long dataSize = (size-bytesSent) > PAYLOAD_SIZE ? PAYLOAD_SIZE : (size - bytesSent);
			memset(buf,0,BUFFER_SIZE);
			//Switch Endian
			d.sequenceNumber = htons(m_sequenceNumber);
			memcpy(buf, &d.sequenceNumber, sizeof(uint16_t));
			//Payload of Packet
			d.payload = (char *)(m_sendBuf + m_sendBufOffset);
			//Copy in the payload
			memcpy(buf + sizeof(uint16_t), (char *)(m_sendBuf + m_sendBufOffset), dataSize);
			//Copy in end of byte
			d.errorDetection = 0;
			m_logFile<<"Frame: "<<frame<<" of packet: "<<m_packetCount<<" sent"<<std::endl;
			d.endOfPacket = ((size - bytesSent) > PAYLOAD_SIZE)? NOT_END_OF_PACKET : END_OF_PACKET_BYTE;
			//Copy in end of packet byte
			memcpy(buf + sizeof(uint16_t) + dataSize, &d.endOfPacket, sizeof(d.endOfPacket));
			if(d.endOfPacket){
				m_logFile<<"Packet: "<<m_packetCount<<" sent"<<std::endl;
			}
			//Error Detection Bytes
#ifndef ARTIFICIAL_ERROR
			d.errorDetection = calculateXorCheck(buf, dataSize + sizeof(uint16_t) + 1);
#else
			transmissionTries++;
			d.errorDetection = ((transmissionTries % 5) ==4)?~calculateXorCheck(buf, dataSize + sizeof(uint16_t) + 1):calculateXorCheck(buf, dataSize + sizeof(uint16_t) + 1);
#endif
			//Copy in end of packet byte
			memcpy(buf + sizeof(uint16_t) + dataSize + 1, &d.errorDetection, sizeof(uint16_t));
			bytesSentDelta = this->writeToSocket(buf, dataSize + 2*sizeof(uint16_t) + 1) - 5;
			//Wait for send ack from server
			goodAck = this->waitForSendAck();
			if(!goodAck){
				m_logFile<<"Frame: "<<frame<<" of packet: "<<m_packetCount<<" being retransmitted"<<std::endl;
			}
		}
		bytesSent += bytesSentDelta;
		m_sendBufOffset += bytesSentDelta;
		//Increment Sequence Number
		frame++;
		//m_sequenceNumber++;
	}
	m_packetCount++;
	return true;
}
///@author sdmichelini
void data_link_layer::flushSendBuffer(){
	//Reset offset and clear memory structure
	//Clear the memory of the buffer
	memset(m_sendBuf, 0, BUFFER_SIZE);
	//Original Buffer Offset is Zero
	m_sendBufOffset = 0;
}
///@author sdmichelini
uint16_t data_link_layer::calculateXorCheck(char * packet, unsigned long packetSize){
	uint16_t result = 0;
	for(unsigned int i = 0; i < (packetSize/2); i++){
		uint16_t curNum = 0;

		//Copy in the current packet
		memcpy(&curNum, (char *)(packet + 2*i), sizeof(uint16_t));

		//XOR fold
		result ^= curNum;
	}
	return result;
}
///@author msbeaulieu
bool data_link_layer::waitForSendAck(){
#ifdef NO_SERVER
	m_sequenceNumber++;
	return true;
#endif
	//Here we use the select system call
	char buf[ACK_FRAME_SIZE];
	if(this->readFromSocketWithTimeout(buf, ACK_FRAME_SIZE, ACK_TIMEOUT) == ACK_FRAME_SIZE){
		//Check Ack
		//Sequence ID, Network-Endian Sequence ID, Error Detection bytes
		uint16_t seqId,nSeqId,error;
		//Copy over the Data
		memcpy(&nSeqId, buf, sizeof(uint16_t));
		//Error
		memcpy(&error, buf, sizeof(uint16_t));
		seqId = ntohs(nSeqId);
		if(seqId == m_sequenceNumber){
			//Increment Sequence Number
			m_logFile<<"Ack Frame:"<<m_sequenceNumber<<" received"<<std::endl;
			m_sequenceNumber++;
			return true;
		}
		else{
			std::cout<<"Ack Error: Sequence ID from Packet: "<<seqId<<" != local sequence number: "<<m_sequenceNumber<<std::endl;
			m_logFile<<"Ack Frame Received in Error"<<std::endl;
			return false;
		}
	}
	else{
		m_logFile<<"Ack Frame: "<<m_sequenceNumber<<" timer expired"<<std::endl;
		return false;
	}
}
///@author msbeaulieu
bool data_link_layer::waitForPacketAck(){
	//Here we use the select system call
	char buf[ACK_PACKET_SIZE];
	if(this->readFromSocketWithTimeout(buf, ACK_PACKET_SIZE, ACK_TIMEOUT) == ACK_PACKET_SIZE){
		m_logFile<<"Ack Packet: "<<m_packetCount<<" received."<<std::endl;
		return true;
	}
	else{
		m_logFile<<"Ack Packet: "<<m_packetCount<<" timer expired."<<std::endl;
		return false;
	}
}
///@author msbeaulieu
data_link_layer::~data_link_layer(){
	m_logFile.close();
}

