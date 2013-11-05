#ifndef StreamConnection_h
#define StreamConnection_h

#include <stdexcept>
#include <vector>
#include <string>

//definitions
#define READ_BUFFER_SIZE 512
#define SEND_CHUNK_SIZE 512

using namespace std;

class StreamConnectionError : public runtime_error {
	public:
		StreamConnectionError(const string& what_arg)
		   	: runtime_error(what_arg)
	   	{}
};

class StreamConnection {
	// definitions
	public:
		enum ConnectionStatus {Connected, Disconnected, Listening};

	protected:
		enum ReadState {Start, ReadingHeader, ReadingContent, Done};

	// methods
	public:
		StreamConnection();
		~StreamConnection();
		virtual void openServer(unsigned short port); // may throw StreamConnectionError
		virtual void acceptConnection();
		virtual void closeServer();

		virtual void connectToHost(string host, unsigned short port);
		virtual void closeConnection();

		virtual ConnectionStatus getStatus();

		virtual vector<unsigned char> readPacket();
		virtual void writePacket(vector<unsigned char> data);

	protected:
		virtual vector<unsigned char> readChunk();
		virtual void mySocketError(string msg); // may throw StreamConnectionError
		virtual void myCloseSocket(int socket);

	// variables
	protected:
		ConnectionStatus status;
		int serverFd;
		int connectionFd;
		vector<unsigned char> readBuffer;
};

#endif // StreamConnection_h
