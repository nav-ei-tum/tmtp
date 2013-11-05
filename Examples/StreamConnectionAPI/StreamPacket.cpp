#include "StreamPacket.h"
#include <limits.h>


void StreamPacket::setContent(vector<unsigned char> data)
{
	if (data.size() > ULONG_MAX - 1) {
		throw StreamPacketError("Content too long");
	} else {
		content = data;
	}
}

vector<unsigned char> StreamPacket::getContent()
{
	return content;
}

unsigned long StreamPacket::getHeaderLength(unsigned char)
{
	return headerLength;
}

unsigned long StreamPacket::getLength(vector<unsigned char> header)
{
	unsigned long length = 0;

	if (header[0] != version) {
		throw StreamPacketError("Wrong version");
	} else {
		length |= header[1] << 24;
		length |= header[2] << 16;
		length |= header[3] << 8;
		length |= header[4];
	}
	return length;
}

vector<unsigned char> StreamPacket::wrap()
{
	vector<unsigned char> raw;

	unsigned long length = content.size() + headerLength;

	raw.push_back(version & 0x00ff);
	raw.push_back((length >> 24) & 0x000000ff);
	raw.push_back((length >> 16) & 0x000000ff);
	raw.push_back((length >> 8) & 0x000000ff);
	raw.push_back(length & 0x000000ff);
	if (content.size() != 0) {
		raw.insert(raw.end(), content.begin(), content.end());
	}

	return raw;
}

void StreamPacket::unwrap(vector<unsigned char> raw)
{
	if (raw.size() < headerLength) {
		throw StreamPacketError("Raw packet too short");
	} else if (raw[0] != version) {
		throw StreamPacketError("Wrong version");
	} else {
		unsigned long length = 0;
		length |= raw[1] << 24;
		length |= raw[2] << 16;
		length |= raw[3] << 8;
		length |= raw[4];
		if (raw.size() != length) {
			throw StreamPacketError("Raw packet has wrong size");
		} else {
			content.assign(raw.begin() + headerLength, raw.end());
		}
	}
}

