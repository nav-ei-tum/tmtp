#ifndef StreamPacket_h
#define StreamPacket_h

#include <vector>
#include <stdexcept>

using namespace std;

class StreamPacketError : public runtime_error {
	public:
		StreamPacketError(const string& what_arg)
		   	: runtime_error(what_arg)
	   	{}
};

class StreamPacket
{
	// definitions
	protected:
		static const unsigned short version = 1;
		static const unsigned short headerLength = 5;

	// methods
	public:
		virtual void setContent(vector<unsigned char> data);
		virtual vector<unsigned char> getContent();

		virtual unsigned long getHeaderLength(unsigned char firstByteOfHeader);
		virtual unsigned long getLength(vector<unsigned char> header);

		virtual vector<unsigned char> wrap();
		virtual void unwrap(vector<unsigned char> raw);

	// variables
	protected:
		vector<unsigned char> content;
};

#endif // StreamPacket_h
