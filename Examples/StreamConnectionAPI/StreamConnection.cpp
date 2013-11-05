#include "StreamConnection.h"
#include "StreamPacket.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>

#ifdef _WIN32
// Windows specific
#pragma comment(lib, "wsock32.lib")
#include <winsock2.h>
typedef int socklen_t;
#else
// POSIX specific
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define INVALID_SOCKET -1
#endif //_WIN32

using namespace std;

StreamConnection::StreamConnection()
{
#ifdef _WIN32
	// Windows specific
	WSADATA SocketsData;
	if (WSAStartup(MAKEWORD(2, 0), &SocketsData) != 0) {
		mySocketError("Error initializing Winsock");
	}
#endif // _WIN32
}

StreamConnection::~StreamConnection()
{
	myCloseSocket(serverFd);
	myCloseSocket(connectionFd);
}

void StreamConnection::openServer(unsigned short port)
{
	struct sockaddr_in serverAddr;

	// create socket
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == INVALID_SOCKET) {
		mySocketError("ERROR opening server socket");
	}

	// fill server address structure
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	// bind socket
	if (bind(serverFd, reinterpret_cast<struct sockaddr*>(&serverAddr),
				sizeof(serverAddr)) < 0) {
		mySocketError("ERROR on binding server socket");
	}

	// listen
	listen(serverFd,5);
	cout << "listening on port " << port << endl;
	status = Listening;
}

void StreamConnection::acceptConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t sockAddrLen = sizeof(clientAddr);

	// wait for connection
	connectionFd = accept(serverFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &sockAddrLen);
	if (connectionFd == INVALID_SOCKET) {
		mySocketError("ERROR on accepting client connection");
	}
	cout << "client connected from ip " << hex << ntohl(clientAddr.sin_addr.s_addr) << endl;
	status = Connected;
}

void StreamConnection::closeServer()
{
	myCloseSocket(serverFd);
	if (status == Listening) {
		status = Disconnected;
	}
}

void StreamConnection::connectToHost(string host, unsigned short port)
{
	struct hostent *server;
	struct sockaddr_in serverAddr;

	cout << "connecting to " << host << ":" << port << endl;

	// create socket
	connectionFd = socket(AF_INET, SOCK_STREAM, 0);
	if (connectionFd == INVALID_SOCKET) {
		mySocketError("ERROR opening client socket");
	}

	// resolve server address
	server = gethostbyname(host.c_str());
	if (server == NULL) {
		throw StreamConnectionError("ERROR, no such host");
	}

	// fill server address struct
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	memcpy(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);
	serverAddr.sin_port = htons(port);

	// connect
	if (connect(connectionFd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
		mySocketError("ERROR connecting");
	}
	cout << "connected" << endl;
	status = Connected;
}

void StreamConnection::closeConnection()
{
	myCloseSocket(connectionFd);
	status = Disconnected;
}

StreamConnection::ConnectionStatus StreamConnection::getStatus()
{
	return status;
}

vector<unsigned char> StreamConnection::readPacket()
{
	StreamPacket packet;
	unsigned long headerLength = 0;
	unsigned long packetLength = 0;
	ReadState state = Start;
	vector<unsigned char> data;

	while (state != Done) {
		if (status == Connected) {
			switch (state) {
				case Start:
					if (readBuffer.size() < 1) {
						vector<unsigned char> chunk = readChunk();
						readBuffer.insert(readBuffer.end(), chunk.begin(), chunk.end());
					} else {
						headerLength = packet.getHeaderLength(readBuffer[0]);
						state = ReadingHeader;
					}
					break;
				case ReadingHeader:
					if (readBuffer.size() < headerLength) {
						vector<unsigned char> chunk = readChunk();
						readBuffer.insert(readBuffer.end(), chunk.begin(), chunk.end());
					} else {
						vector<unsigned char> header (readBuffer.begin(), readBuffer.begin() + headerLength);
						packetLength = packet.getLength(header);
						state = ReadingContent;
					}
					break;
				case ReadingContent:
					if (readBuffer.size() < packetLength) {
						vector<unsigned char> chunk = readChunk();
						readBuffer.insert(readBuffer.end(), chunk.begin(), chunk.end());
					} else {
						vector<unsigned char> raw (readBuffer.begin(), readBuffer.begin() + packetLength);
						readBuffer.erase(readBuffer.begin(), readBuffer.begin() + packetLength);
						try {
							packet.unwrap(raw);
							data = packet.getContent();
						} catch (StreamPacketError &e) {
							ostringstream msg;
							msg << "Error in StreamPacket: " << e.what() << endl;
							throw StreamConnectionError(msg.str());
						}
						state = Done;
					}
					break;
				default:
					throw StreamConnectionError("Error in reading packet state machine: undefined state");
					break;
			}
		} else {
			//throw StreamConnectionError("Connection closed");
			break;
		}
	}

	return data;
}

void StreamConnection::writePacket(vector<unsigned char> data)
{
	StreamPacket packet;
	vector<unsigned char> raw;

	try {
		packet.setContent(data);
		raw = packet.wrap();
	} catch (StreamPacketError &e) {
		ostringstream msg;
		msg << "Error in StreamPacket: " << e.what() << endl;
		throw StreamConnectionError(msg.str());
	}

	if (status == Connected) {
		while (raw.size() > 0) {
			vector<unsigned char> chunk;
			if (raw.size() > SEND_CHUNK_SIZE) {
				chunk.assign(raw.begin(), raw.begin() + SEND_CHUNK_SIZE);
				raw.erase(raw.begin(), raw.begin() + SEND_CHUNK_SIZE);
			} else {
				chunk = raw;
				raw.clear();
			}
			char buffer[SEND_CHUNK_SIZE];
			for (unsigned short i = 0; i < chunk.size(); i++) {
				buffer[i] = chunk[i];
			}
			int n = send(connectionFd, buffer, chunk.size(), 0);
			if (n < 0) {
				mySocketError("ERROR writing to socket");
			}
		}
	} else {
		throw StreamConnectionError("Connection closed");
	}
}

vector<unsigned char> StreamConnection::readChunk()
{
	vector<unsigned char> chunk;

	if (status == Connected) {
		char buffer[READ_BUFFER_SIZE];
		int n = recv(connectionFd, buffer, READ_BUFFER_SIZE, 0);
		if (n < 0) {
			mySocketError("ERROR reading from socket");
		} else if (n == 0) {
			closeConnection();
			//throw StreamConnectionError("Connection reset by peer");
		} else {
			chunk.assign(buffer, buffer + n/sizeof(unsigned char));
			//cout << "Received bytes: " << dec << chunk.size() << endl;
		}
	} else {
		throw StreamConnectionError("Connection closed");
	}

	return chunk;
}

void StreamConnection::mySocketError(string msg)
{
#ifdef _WIN32
	// Windows specific
	ostringstream newMsg;
	newMsg << msg << ": Error code " << WSAGetLastError();
	msg = newMsg.str();
#else
	// POSIX specific
	int currentError = errno;
	msg.append(": ");
	//FIXME strerror is not thread save
	msg.append(strerror(currentError));
#endif // _WIN32
	throw StreamConnectionError(msg);
}

void StreamConnection::myCloseSocket(int socket)
{
#ifdef _WIN32
	// Windows specific
	closesocket(socket);
#else
	// POSIX specific
	close(socket);
#endif // _WIN32
}

/* TODO:
 * - handle resets by peer correctly
 * - thread save alternative for strerror
 * - resync if a problem occured while reading a packet
 * - multiple connections handled with select
 * - check if send buffer is full
 */
