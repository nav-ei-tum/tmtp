/**
        Copyright 2013 Institute for Communications and Navigation, TUM

        This file is part of tmtp.

tmtp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

tmtp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tmtp. If not, see <http://www.gnu.org/licenses/>.
*/
#include "SpacePacketConf.h"
#include <iostream>
#include <stdint.h>

using namespace std;

// Constructor
SpacePacketConf::SpacePacketConf()
{
}

// Receives a packet header (only the first Byte) and checks if it describes an idle packet.
bool SpacePacketConf::isIdlePacket(uint8_t firstByteOfHeader)
{
	// doesn't detect the real idle packet but is good enough for the test
	uint16_t packetVersion = (firstByteOfHeader >> 5) & 0x0007;	// Extracts the three most significant bits.
	return (packetVersion == idlePacketVersion);	// returns TRUE if the extracted packet version is eq. to 001
}

// Extracts and calculates the total packet length (header length + message length + 1).
uint64_t SpacePacketConf::extractPacketLength(vector<uint8_t> header)
{
	uint64_t length = (header[4] << 8) | header[5];	// The 5th and 6th Bytes of the header are stored in variable "length".
	return length + 1 + packetHeaderLength;					// Returns the TOTAL packet length +1.
}

// Returns the hardcoded value of variable packetHeaderLength.
uint64_t SpacePacketConf::getPacketHeaderLength(uint8_t)
{
	// It received an uint8_t but does absolutely nothing with it.
	return packetHeaderLength;	// It returns a static const uint16_t as uint64_t!
}

// Generates an idle packet for testing purposes.
uint8_t SpacePacketConf::genIdlePacket()
{
	// Not a real idle packet but good enough for the test. 
	// Returns uint8_t idlePacket = 0x20 (00100000 in binary).
	return idlePacket;
}

// Appends a 6-Byte header to a message (upper-layer data) to generate a space packet for testing.
vector<uint8_t> SpacePacketConf::genTestPacket(vector<uint8_t> message)
{
	vector<uint8_t> packet;
	uint16_t dataSize = message.size() - 1;

	packet.push_back(0x00);						// First Byte	- Packet ID			0000 0000
	packet.push_back(0x00);						// Second Byte	- Sequence Control	0000 0000
	packet.push_back(0xC0);						// Third Byte	- Message Length	1100 0000 
	packet.push_back(0x00);										// Fourth Byte	0000 0000
	packet.push_back((dataSize >> 8) & 0x00FF);	// First half of dataSize		XXXX XXXX
	packet.push_back(dataSize & 0x00FF);		// Second half of dataSize		XXXX XXXX

	packet.insert(packet.end(),message.begin(),message.end());	// After the 6th header Byte, 
																// the whole message is inserted

	return packet;
}

// Dissects a packet into its components and displays them as messages for debugging.
void SpacePacketConf::packetDebugOutput(vector<uint8_t> packet)
{
	// Here we establish the packet header components. For a summary of these, check the comments in "SpacePacketConf.h"
	if (packet.size() >= packetHeaderLength) {												// If the packet is more than just a header:
		uint16_t id = (packet[0] << 8) | packet[1];									// The packet ID is extracted.
		uint16_t sequenceControl = (packet[2] << 8) | packet[3];						// The sequence control is extracted.
		uint64_t length = ((packet[4] << 8) | packet[5]) + 1 + packetHeaderLength;		// The total packet length is calculated.

		// From the packet ID, the following are extracted:
		uint16_t version = (id >> 13) & 0x0007;			// The packet version (3 bits).
		uint16_t type = (id >> 12) & 0x0001;				// The packet type (1 bit).
		bool dataFieldHeaderPresent = (id >> 11) & 0x0001;		// The data field header flag (1 bit).
		uint16_t apid = id & 0x07FF;						// The application ID (11 bits).
		
		// From the sequence control, the following are extracted:
		uint16_t groupingFlags = (sequenceControl >> 14) & 0x0003;	// The grouping flags (2 bits).
		uint16_t sourceSequenceCount = sequenceControl & 0x3FFF;		// The source sequence counter (10 bits).
		
		// Here we establish the DATA packet contents/headers components. For a summary of these, check the comments in "SpacePacketConf.h"
		uint16_t pusVersion = 0, pusServiceType = 0, pusServiceSubtype = 0, pusPacketSubcounter = 0;
		if (dataFieldHeaderPresent) {		
			pusVersion = (packet[6] >> 4) & 0x07;
			pusServiceType = packet[7];
			pusServiceSubtype = packet[8];
			pusPacketSubcounter = packet[9];
		}

		// The extracted components of the packet are displayed.
		if (packet.size() >= length) {
			vector<uint8_t> content(packet.begin()+packetHeaderLength, packet.begin()+length);

			cout << "Space Packet[" << dec << length << "] ";
			cout << "Ver: " << dec << version << ", ";
			cout << "Typ: " << dec << type << ", ";
			cout << "DFH: " << boolalpha << dataFieldHeaderPresent << ", ";
			cout << "APID: " << dec << apid << ", ";
			cout << "GF: " << dec << groupingFlags << ", ";
			cout << "SSC: " << dec << sourceSequenceCount << ", ";
			if (dataFieldHeaderPresent) {
				cout << "PUSv: " << dec << pusVersion << ", ";
				cout << "PUSst: " << dec << pusServiceType << ", ";
				cout << "PUSsst: " << dec << pusServiceSubtype << ", ";
				cout << "PUSpsc: " << dec << pusPacketSubcounter << ", ";
			}
			/*cout << "Content: \"";
			for (unsigned short i=0; i < content.size(); i++) {
				cout << content[i];
			}
			cout << "\" [" << dec << content.size() << "]";*/
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
