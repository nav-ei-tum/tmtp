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
#include "NetProtConf.h"
#include <vector>
#include <iostream>
#include <stdint.h>

using namespace std;

NetProtConf::NetProtConf()
{
}

bool NetProtConf::isIdlePacket(uint8_t)
{
	// Marks all packets as idle
	return true;
}

uint64_t NetProtConf::extractPacketLength(vector<uint8_t>)
{
	// Idle packets have a length of 1.
	return 1;
}

uint64_t NetProtConf::getPacketHeaderLength(uint8_t)
{
	// Idle packets have a length of 1 and contain only a header.
	return 1;
}

uint8_t NetProtConf::genIdlePacket()
{
	// The actual idle packet content is unimportant.
	return '*';
}

vector<uint8_t> NetProtConf::genTestPacket(vector<uint8_t>)
{
	// The test packet is also an idle packet
	return vector<uint8_t>(1,this->genIdlePacket());
}

void NetProtConf::packetDebugOutput(vector<uint8_t> packet)
{
	// uses std::dec to display packet.size() in decimal notation
	cout << "IdlePacket[" << dec << packet.size() << "] ";
	cout << "Content: \"";
	
	for (uint16_t i=0; i < packet.size(); i++) {
		cout << packet[i];
	}
	cout <<  endl;
}
