#include "tmtp/Tmtp.h"
#include "tmtp/TmtpOcf.h"
#include "tmtp/TmtpPacket.h"

#include "ticp/TicpServer.hpp"

#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

/*!	\brief Creates a physical-, master- and one virtual channel to transmit packets read from a binary file to be transmitted using the TMTP. */
class TmSender : private ticp::Server<ticp::data_v2_t>
{
	/* methods */
	public:
/*!	\brief Constructor of the TmSender class. 
 *
 * Establishes the following values as default:
 *	- TMTP Frame Length = 1115 Bytes.
 *	- Spacecraft ID = 102.
 *	- Data source file name = "inputFile".
 *
 * Creates a physical channel with a frame length of 1115 Bytes and enables Frame Error Control Field (i.e. CRC will be used). \n
 * Through the physical channel, a master channel is defined and inherits its settings. \n
 * Through the master channel, the 2nd virtual channel is defined and inherits its settings. \n
 * Configures packets in virtual channel 2 as specified in TestProtConf. \n
 * Opens a file called "inputFile", reads and copies the contents of the input file bytewise into a vector of Bytes called fileContent. \n
 * Appends a 2-Byte header to fileContent to make it a packet and places it in the output queue of Virtual Circuit 2. \n
 * Finishes by starting the TICP server listening on port 1736.
 */
		TmSender();

	private:
/*!	\brief Generates a TICPv2 Frame. 
 *	\param timestamp TMTP timestamp appended to the Frame.
 */
		ticp::data_v2_t genFrame(ticp::Timestamp timestamp);

	/* attributes */
	private:
		TmPhysicalChannel* sendPc;		/*!< Transmitting Physical Channel. */

};

// Constructor of the TmSender class.
TmSender::TmSender()
{
	unsigned short frameLength = 223*5;		// TMTP Frame Length = 1115 Bytes.
	unsigned short scid = 102;				// Spacecraft ID = 102.
	string inFileName = "inputFile";

	/* configure protocol */
	TestProtConf conf;
	
	/* physical channel */
	sendPc = new TmPhysicalChannel(frameLength);	// Creates a physical channel with a frame length of 1115 Bytes.
	sendPc->activateFecf();							// Frame Error Control Field enabled (i.e. CRC will be used).
	
	/* master channel */
	TmMasterChannel *sendMc = sendPc->createTmMasterChannel(scid);	// Through the physical channel, a master channel is defined and inherits its settings.
	sendMc->deactivateOcf();
	
	/* virtual channel 1 - data */
	TmVirtualChannel *sendVc1 = sendMc->createTmVirtualChannel(1);	// Through the master channel, the 2nd virtual channel is defined and inherits its settings.
	sendVc1->setNetProtConf(&conf);									// Configures VC2 packets as specified in TestProtConf.
	
	/* read file to transfer */
	vector<unsigned char> fileContent;		
	ifstream inFile(inFileName.c_str());	// Opens a file called "inputFile".
	unsigned char c = inFile.get();		// Reads the 1st Byte in the input file.
	while (inFile.good()) {				// While the end of the file has not been reached and no error occurred:
		fileContent.push_back(c);			// Places the contents of the input file bytewise into this vector.
		c = inFile.get();					// Reads the next Byte in the input file.
	}
	sendVc1->sendPacket(conf.genTestPacket(fileContent));	// Appends a 2-Byte header to fileContent to make it a packet and 
															// places it in the output queue of Virtual Circuit 2.
	
	/* start TICP server */
	ticpConnectGetFrameFunction(boost::bind(&TmSender::genFrame, this, ticp::Timestamp()));
	ticpConnectGetTimedFrameFunction(boost::bind(&TmSender::genFrame, this, _1));

	const unsigned short port = 1736;
	cout << "Server listening on port " << port << endl;
	ticpStartServer(port);
}

// Generates a TICPv2 Frame.
ticp::data_v2_t TmSender::genFrame(ticp::Timestamp timestamp)
{
	ticp::data_v2_t frame;
	const ticp::type_t type = 0;
	ticp::data_v1_t data;
	
	// Convert from TICP to TMTP timestamp
	TmFrameTimestamp t;
	t.setSeconds(timestamp.getSeconds());
	t.setFractions(timestamp.getFraction());
	
	try {
		data = sendPc->sendFrame(t);
	} catch (TmPhysicalChannelError& e) {
		cout << "Error in TmPhysicalChannel: " << e.what() << endl;
	}

	// FIXME only send data over TICPv2 now, make use of metadata
	frame[type] = data;
	return frame;
}

int main()
{
	try {
		TmSender sender;	// Creates a physical-, master- and one virtual channel (1). 
							// Reads a binary file, encapsulates its contents into a packet and 
							// places it the virtual channel output queue.
	} catch (TmTransferFrameError& e){
		cout << "Error in TmTransferFrame: " << e.what() << endl;
	} catch (TmOcfError& e){
		cout << "Error in TmOcf: " << e.what() << endl;
	} catch (TmVirtualChannelError& e){
		cout << "Error in TmVirtualChannel " << e.getVcid() << ": " << e.what() << endl;
	} catch (TmMasterChannelError& e){
		cout << "Error in TmMasterChannel: " << e.what() << endl;
	} catch (TmPhysicalChannelError& e){
		cout << "Error in TmPhysicalChannel: " << e.what() << endl;
	} catch (GroundPacketServerError& e){
		cout << "Error in GroundPacketServer: " << e.what() << endl;
	}

	return 0;
}
