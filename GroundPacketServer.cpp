#include "TmFrameTimestamp.h"
#include "GroundPacketServer.h"
#include "TmVirtualChannel.h"
#include "NetProtConf.h"
#include "myErrors.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <stdint.h>
#include <sys/stat.h>

using namespace std;

// Constructor of the GroundPacketServer class inherits from the PacketServer class.
GroundPacketServer::GroundPacketServer(NetProtConf *conf) : PacketServer(conf)
{
	debugOutput = false;	// Debug output display deactivated.
}

// Retrieves a packet from the VC input queue and stores it in a binary file under "outputFiles/".
void GroundPacketServer::signalNewPacket()
{
	if (tmVc) {								// If a virtual channel has been defined.
		while (tmVc->packetAvailable()) {	// ... And while there are packets waiting in the input queue:
			try {
				TimeTaggedPacket packetAndTimestamp = tmVc->receivePacket();	// Retrieves the joint data structure with the packet and timestamp from the queue.
				TmFrameTimestamp packetTimestamp;						// Extracts the timestamp from the data structure,
				vector<uint8_t> packet = packetAndTimestamp.data;		// Extracts the packet from the data structure,
				if (debugOutput) {										// and shows its contents (if debug output is activated).
					cout << "Received " << flush;
					netProtConf->packetDebugOutput(packet);
				}
				ostringstream os;
				mkdir ("outputFiles", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);		// Creates directory "outputFiles" with 
													// read/write/search permissions for owner
													// and group, and with read/search
													// permissions for others.
				if (packetAndTimestamp.timestamp.isValid()){
					os << "outputFiles/" << packetAndTimestamp.timestamp.getSeconds();	// Creates a string (to be used as a path) with the packet timestamp.
				}
				else {
					os << "outputFiles/" << time(NULL);						// Creates a string (to be used as a path) with the creation time.
				}
				ofstream file(os.str().c_str(), ofstream::binary);			// Creates a binary file in folder "outputFiles/". The file name is the time of creation.
				if (!file) {
					throw GroundPacketServerError(string("Failed to open ") + os.str() + " for writing.");
				}
				for (size_t n = 2; n < packet.size(); n++) {
					file.put(packet[n]);			// Stores the contents of the packet (excluding the 1st two header Bytes) in the binary file.
				}
			} catch (TmVirtualChannelError& e) {
				ostringstream error;
				error << "Error in TmVirtualChannel: " << e.what() << endl;
				throw GroundPacketServerError(error.str());
			}
		}
	} else {
		ostringstream error;
		error << "No TmVirtualChannel specified." << endl;
		throw GroundPacketServerError(error.str());
	}
}

// Sets the debug output flag to TRUE.
void GroundPacketServer::activateDebugOutput()
{
	debugOutput = true;
}

// Sets the debug output flag to FALSE.
void GroundPacketServer::deactivateDebugOutput()
{
	debugOutput = false;
}
