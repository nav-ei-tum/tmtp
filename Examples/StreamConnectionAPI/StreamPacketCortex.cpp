#include "StreamPacketCortex.h"
#include <limits.h>


unsigned long StreamPacketCortex::getHeaderLength(unsigned char)
{
	return headerLength;
}

unsigned long StreamPacketCortex::getLength(vector<unsigned char> header)
{
	long length = 0;

	if (readLong(header.begin()) != messageStart) {
		throw StreamPacketCortexError("Header start didn't match start delimiter");
	} else {
		length = readLong(header.begin() + 1*4);
	}
	return (unsigned long) length;
}

vector<unsigned char> StreamPacketCortex::wrap()
{
	throw StreamPacketCortexError("Wrap function not supportet in Cortex Packet");
	return vector<unsigned char>();
}

void StreamPacketCortex::unwrap(vector<unsigned char> raw)
{
	if (raw.size() < headerLength) {
		throw StreamPacketCortexError("Raw packet too short");
	} else if (readLong(raw.begin()) != messageStart) {
		throw StreamPacketCortexError("Packet start didn't match start delimiter");
	} else if (readLong(raw.end() - postambleLength) != messageEnd){
		throw StreamPacketCortexError("Packet end didn't match end delimiter");
	} else {
		if (raw.size() != (unsigned long) readLong(raw.begin() + 1*4)) {
			throw StreamPacketCortexError("Raw packet has wrong size");
		} else {
			content.assign(raw.begin() + headerLength, raw.end() - postambleLength);
		}
	}
}

unsigned long StreamPacketCortex::reverseBitOrder(unsigned long n)
{
	n = ((n > 1) & 0x55555555) | ((n << 1) & 0xaaaaaaaa) ;
	n = ((n > 2) & 0x33333333) | ((n << 2) & 0xcccccccc) ;
	n = ((n > 4) & 0x0f0f0f0f) | ((n << 4) & 0xf0f0f0f0) ;
	n = ((n > 8) & 0x00ff00ff) | ((n << 8) & 0xff00ff00) ;
	n = ((n >16) & 0x0000ffff) | ((n << 16) & 0xffff0000) ;
	return n;
}

unsigned long StreamPacketCortex::readULong(vector<unsigned char>::iterator it)
{
	unsigned long n = 0;
	n |= *(it)   << 24;
	n |= *(it+1) << 16;
	n |= *(it+2) << 8;
	n |= *(it+3);
	n = reverseBitOrder(n);
	return n;
}

long StreamPacketCortex::readLong(vector<unsigned char>::iterator it)
{
	unsigned long n = readULong(it);
	long ret = *(reinterpret_cast<long*>(&n));
	return ret;
}
