#include "TestProtConf.h"
#include <iostream>
#include <stdint.h>

using namespace std;

// Constructor for the TestProtConf class.
TestProtConf::TestProtConf()
{
}

// Receives a packet header (only the first Byte) and checks if it describes an idle packet.
bool TestProtConf::isIdlePacket(uint8_t firstByteOfHeader)
{
	uint16_t packetVersion = (firstByteOfHeader >> 5) & 0x0007;	// Extracts the three most significant bits.
	return (packetVersion == idlePacketVersion);	// returns TRUE if the extracted packet version is eq. to 000
}

// Extracts and calculates the total packet length (header length + message length + 1).
uint64_t TestProtConf::extractPacketLength(vector<uint8_t> header)
{
	uint16_t tmp = (header[0] << 8) | header[1];	// The whole content of the header is stored in temporary variable "tmp".
	return (tmp & 0x1FFF);								// The 13 least significant bits are take as the packet length.
}

// Returns the hardcoded value of variable packetHeaderLength.
uint64_t TestProtConf::getPacketHeaderLength(uint8_t)
{
	// It received an uint8_t but does absolutely nothing with it.
	return packetHeaderLength;	// It returns a static const uint16_t as uint64_t!
}

// Generates an idle packet for testing purposes.
uint8_t TestProtConf::genIdlePacket()
{
	return idlePacket;		// Returns the predefined idlePacket = 0x1F (0001 1111 in binary).
}

// Appends a 2-Byte header to a message (upper-layer data) to generate a space packet for testing.
vector<uint8_t> TestProtConf::genTestPacket(vector<uint8_t> message)
{
	vector<uint8_t> packet;
	uint16_t header = 0;

	header |= (testPacketVersion & 0x0007) << 13;				// The packet VERSION is "encoded" in the first 3 bits:
																// header = header -OR- [(testPacketHeader -AND- 0000 0000 0000 0111) shifted 13 spaces to the left]
																// Therefore header = 0100 0000 0000 0000

	header |= (message.size() + packetHeaderLength) & 0x1FFF;	// The packet LENGTH is "encoded" in the last 13 bits:
																// header = header -OR- [(message.size + 2) -AND- 0001 1111 1111 1111]

	packet.push_back(header >> 8 );								// The first header Byte is stored in the first position of the packet vector
	packet.push_back(header & 0x00FF);							// Here, the second Byte on the next position
	packet.insert(packet.end(),message.begin(),message.end());	// At the third element of the vector (i.e. after the header) it inserts the whole message vector.

	return packet;												// The packet has now a header with packet version and length.
}

// Dissects a packet into its components and displays them as messages for debugging.
void TestProtConf::packetDebugOutput(vector<uint8_t> packet)
{
	if (packet.size() >= packetHeaderLength) {					// If the packet is more than just a header:
		uint16_t header = (packet[0] << 8) | packet[1];	// The header is extracted.
		uint16_t version = (header >> 13) & 0x0007;		// The packet version is extracted.
		uint16_t length = header & 0x1FFF;				// The joint length of message and header is extracted.
		
		if (packet.size() >= length) {	// The message is extracted.
			vector<uint8_t> content(packet.begin()+packetHeaderLength, packet.begin()+length);

			// The extracted components of the packet are displayed.
			cout << "Test Packet[" << dec << length << "] ";
			cout << "Ver: " << version << ", ";
			cout << "Content: \"";
			for (uint16_t i=0; i < content.size(); i++) {
				cout << content[i];
			}
			cout << "\" [" << dec << content.size() << "]";
			cout <<  endl;
			if (packet.size() > length) {
				cout << "Warning: Packet is too long." << endl;
			}
		} else {
			cout << "Error: Packet is too short." << endl;
		}
	} else {
		cout << "Error: Packet is too short." << endl;
	}
}
