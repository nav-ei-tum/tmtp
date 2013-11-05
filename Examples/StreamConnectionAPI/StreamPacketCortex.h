#ifndef StreamPacketCortex_h
#define StreamPacketCortex_h

#include "StreamPacket.h"
#include <vector>
#include <stdexcept>

using namespace std;

class StreamPacketCortexError : public StreamPacketError {
	public:
		StreamPacketCortexError(const string& what_arg)
		   	: StreamPacketError(what_arg)
	   	{}
};

class StreamPacketCortex : public StreamPacket {
	// definitions
	protected:
		static const unsigned short headerLength = 12;
		static const unsigned short postambleLength = 4;
		static const long messageStart = 1234567890;
		static const long messageEnd = -1234567890;

	// methods
	public:
		virtual unsigned long getHeaderLength(unsigned char firstByteOfHeader);
		virtual unsigned long getLength(vector<unsigned char> header);

		virtual vector<unsigned char> wrap();
		virtual void unwrap(vector<unsigned char> raw);

	protected:
		virtual unsigned long reverseBitOrder(unsigned long n);
		virtual unsigned long readULong(vector<unsigned char>::iterator it);
		virtual long readLong(vector<unsigned char>::iterator it);

};

#endif // StreamPacketCortex_h
