#include "StreamConnection.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

using namespace std;

int main () {
	StreamConnection stream;
	stream.openServer(3000);
	while (true) {
		stream.acceptConnection();
		while (stream.getStatus() == StreamConnection::Connected) {
			vector<unsigned char> data = stream.readPacket();
			if (data.size() > 0) {
				ostringstream retMsg;
				retMsg << "received packet of length " << dec << data.size();
				string retStr = retMsg.str();
				cout << retStr << endl;
				stream.writePacket(vector<unsigned char> (retStr.c_str(), retStr.c_str() + retStr.size()));

				if (data[0] == 'b' && data[1] == 'y' && data.size() == 2) {
					stream.closeConnection();
				}
			}
		}
	}
	stream.closeConnection();
	stream.closeServer();
	return 0;
}
