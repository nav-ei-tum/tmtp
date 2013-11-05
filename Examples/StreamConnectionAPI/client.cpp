#include "StreamConnection.h"
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>

using namespace std;

int main () {
	StreamConnection stream;
	stream.connectToHost("localhost", 3000);

	while (true) {
		string str;
	   	cout << "Packet length to send: ";
	   	getline (cin,str);
		unsigned long size = strtoul(str.c_str(), NULL, 0);

		vector<unsigned char> sendMsg (size, 'o');
		stream.writePacket(sendMsg);

		vector<unsigned char> recMsg = stream.readPacket();
		if (recMsg.size() > 0) {
			cout << "received: ";
			for (unsigned short i=0; i < recMsg.size(); i++) {
				cout << recMsg[i];
			}
			cout << endl;
		}
	}
	return 0;
}
