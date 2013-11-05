#include <tmtp/Tmtp.h>
#include <tmtp/TmtpOcf.h>
#include <tmtp/TmtpPacket.h>

#include <ticp/TicpClient.hpp>

#include <iostream>
#include <vector>
#include <ctime>

using namespace std;

/*!	\brief Defines a physical-, master- and one virtual channel through which packets will be received - 
 * A packet- and an OFC-sink will host the received messages. */
class TmReceiver : private ticp::Client<ticp::data_v2_t>
{
	/* methods */
	public:
	
/*!	\brief  Constructor for the TmReceiver class.
 *
 * Establishes the following values as default:
 *	- TMTP Frame Length = 1115 Bytes.
 *	- Spacecraft ID = 102.
 *
 * Defines a physical channel with a frame length of 1115 Bytes and enables Frame Error Control Field (i.e. CRC will be used). \n
 * Defines an application server (for a ground station) to receive and process OCF messages. \n
 * Through the physical channel, a master channel is defined and inherits its settings. \n
 * Links the Ground OCF server to the master channel and defines it as its OCF sink. \n
 * Through the master channel, the 2nd virtual channel is defined and inherits its settings. \n
 * Defines the format of packets received in virtual channel 2 as specified in TestProtConf. \n
 * Defines an application server (for a ground station) to receive and process packets formatted with TestProtConfm. 
 * This application server will be the sink where received packets will be placed. \n
 * Activates debug messages in the Ground Packet Server. \n
 * Attempts to establish a TICP connection to the local host on port 1739. \n
 * Finishes by listening on port 1736 for TICP frames in an eternal loop. \n
 */
		TmReceiver();
};

//	
TmReceiver::TmReceiver()
{
	unsigned short frameLength = 223*5;		// TMTP Frame Length = 1115 Bytes.
	unsigned short scid = 102;				// Spacecraft ID = 102.

	/* configure protocol */
	TestProtConf conf;

	/* physical channel */
	TmPhysicalChannel *recPc = new TmPhysicalChannel(frameLength);	// Defines a physical channel with a frame length of 1115 Bytes.
	recPc->activateFecf();											// Frame Error Control Field enabled (i.e. CRC will be used).
//	GroundOcfServer *gocfserver = new GroundOcfServer();			// Defines an application server (for a ground station) to receive and process OCF messages.

	/* master channel */
	TmMasterChannel *recMc = recPc->createTmMasterChannel(scid);	// Through the physical channel, a master channel is defined and inherits its settings.
	recMc->deactivateOcf();

//	gocfserver->connectTmMc(recMc);									// Links the Ground OCF server to the master channel.
//	recMc->connectOcfSink(gocfserver);								// Establishes the sink where OCF messages are received.


	/* virtual channel 1 - data */
	TmVirtualChannel *recVc1 = recMc->createTmVirtualChannel(1);	// Through the master channel, the 2nd virtual channel is defined and inherits its settings.
	recVc1->setNetProtConf(&conf);									// Configures VC2 packets as specified in TestProtConf.
	GroundPacketServer *ground1 = new GroundPacketServer(&conf);	// Defines an application server (for a ground station) to receive and process packets formatted with TestProtConf.
	ground1->connectTmVc(recVc1);									// Establishes Virtual Channel 2 as the packet source for the Ground Packet Server.
	recVc1->connectPacketSink(ground1);								// Establishes the sink where received packets will be placed.
	ground1->activateDebugOutput();									// Activates debug messages in the Ground Packet Server.


	/* connect to receive TM sync module */
	const string hostname("localhost");
	const unsigned short port = 1736;
	cout << "Trying to connect to server at " << hostname;
	cout << " on port " << port << " ... " << flush;
	ticpConnectBlocking(hostname, port, 0);							// Attempts to establish a TICP connection to the local host on port 1739.
	//ticpConnect("localhost", 1736);
	cout << "successful" << endl;

	while (true) {													// Eternaly listens for TICP frames and pops warnings.

		ticp::data_v2_t frame;			// We create a TICP frame.
		ticpGetFrameBlocking(frame);	// Some magic happens here and the variable "frame" is filled with a TMTP frame, a timestamp and a bitrate.

		/* = The following is experimental =
		Assuming the format of ticp::data_v2_t is an array of vectors of Bytes, we would have three such vectors. 
		Each of array index (in hex) corresponds to specific content:
		0x00 = A TMTP frame.
		0x01 = A timestamp.	// This guy has to somehow provide TWO varables: seconds (uint64_t) and fractions (double, i.e. 8 Bytes long).
		0x02 = A bitrate.
		
	
		vector <uint8_t> frame_data;	// We create a vector of Bytes which will receive the frame data.
			frame_data.insert(frame_data.end(), frame[0x00].begin(), frame[0x00].size());	// We copy the contents of frame[0x00] into our vector of Bytes.
		
		TMTP::TmFrameTimestamp frame_timestamp;		// We create an instance of TmFrameTimestamp.
			// Given the fact a timestamp has two parts; one 64- and the other 32-bit long:
			// FIRST - We take the contents of the 1st eight Bytes of frame[0x01] and put them together to assemble an uint64_t.
			for (int i = 0 ; i < 8 ; i++) {
				uint64_t secs |= (frame[0x01][i] << (56-(8*i)))
			}
			frame_timestamp.setSeconds(secs);		// We assign our assembled variable as the seconds.
			
			// SECOND - We take the contents of last four Bytes of frame[0x01] and put them together to assemble a float (which is 4 Bytes long).
			for (int i = 8 ; i < 12 ; i++) {
				float fracs |= (frame[0x01][i] << (88-(8*i)))
			}
			frame_timestamp.setFractions(fracs);	//  We assign our assembled variable as the fractions.
			
		TMTP::TmFrameBitrate frame_bitrate;		// We create an instance of TmFrameBitrate.
			// We take the contents of each byte in frame[0x02] and put them together to assemble a double float (which is 8 Bytes long).
			for (int i = 0 ; i < 8 ; i++) {
				double bps |= (frame[0x02][i] << (56-(8*i)))
			}
			frame_bitrate.setBitrate(bps);	// We assign our assembled variable as the bitrate.
		
		// Now we pass all three elements to the receiveFrame function of the Physical Channel:
		TmChannelWarning warning = recPc->receiveFrame(frame_data, frame_timestamp, frame_bitrate);
		*/

		// But until all that is implemented, we will use the following metadata:
		
		TmFrameTimestamp frame_timestamp;
		TmFrameBitrate frame_bitrate;
		
		frame_timestamp.setSeconds(static_cast<uint64_t>(time(0)));
		frame_timestamp.setFractions(0.1234);
		frame_bitrate.setBitrate(64000.1234);
		TmChannelWarning warning = recPc->receiveFrame(frame[0], frame_timestamp, frame_bitrate);

		/* NOTE: We are using real system time as the timestamp seconds. We get this from "time(0)", the returned value is of type "time_t".
		Unix and POSIX-compliant systems implement the time_t type as a signed integer (typically 32 or 64 bits wide) 
		which represents the number of seconds since the start of the Unix epoch: midnight UTC of January 1, 1970 (not counting leap seconds). */
		
		//TmChannelWarning warning = recPc->receiveFrame(frame);	// THIS WAS THE ORIGINAL: only uses the data and ignores the metadata. FIXME: Make use of TICPv2

		ground1->signalNewPacket();		// Stores received packets inside "outputFiles/"

		while (warning.warningAvailable()) {
			cout << warning.popWarning() << endl;
		}

	}
}

int main()
{

	try {
		TmReceiver receiver;	// Defines a physical-, master- and one virtual channel (1) through packets will be received. 
								// A packet- and an OFC-sink will host the received messages.
								// A TICP connection will be attempted on port 1736.
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
