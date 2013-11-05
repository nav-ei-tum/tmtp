#include "TmPhysicalChannel.h"
#include "TmMasterChannel.h"
#include "TmTransferFrame.h"
#include "TmFrameTimestamp.h"
#include "TmFrameBitrate.h"
#include "myErrors.h"

#include "ticp/TicpTimestamp.hpp"

#include <vector>
#include <iostream>
#include <sstream>
#include <stdint.h>

using namespace std;

// Constructor of the TmPhysicalChannel class.
TmPhysicalChannel::TmPhysicalChannel(uint16_t length)
{
	if ((length >= 7) && (length <= 2048)) {	// Total frame length must be min. 7, max. 2048 Bytes long.
		frameLength = length;
	} else {
		ostringstream error;
		error << "Frame length out of range (7-2048)." << endl;
		throw TmPhysicalChannelError(error.str());
	}
	
	// By default, sets the following flags:
	fecfPresent = false;	// No Frame Error Control Field present.
	masterChannel = NULL;	// The generated master channel pointer is intialized to NULL.
}

// Destructor of the TmPhysicalChannel class.
TmPhysicalChannel::~TmPhysicalChannel(void)
{
	delete masterChannel;	// Removes the generated master channel from memory.
}

// Retrieves the total frame length value.
uint16_t TmPhysicalChannel::getFrameLength()
{
	return frameLength;
}

// Sets the FECF Flag to TRUE.
void TmPhysicalChannel::activateFecf()
{
	fecfPresent = true;
}

// Sets the FECF Flag to FALSE.
void TmPhysicalChannel::deactivateFecf()
{
	fecfPresent = false;
}

// Retrieves the value of the  FECF Flag.
bool TmPhysicalChannel::getFecfStatus()
{
	return fecfPresent;
}

// Creates a master channel bound to this physical channel.
TmMasterChannel* TmPhysicalChannel::createTmMasterChannel(uint16_t scid)
{
	delete masterChannel; // Frees any memory previously allocated to the master channel pointer.
	try {
		masterChannel = new TmMasterChannel(scid, this);	// Creates a master channel with the following attributes:
		// 	- Spacecraft ID = scid;
		// 	- ocfPresent = true;
		// 	- sendFrameCount = 0;
		// 	- recFrameCount = 0;
		// 	- secondHeaderPresent = false;
		// 	- extendedVcFrameCountUsed = false;
		// 	- ocfSink = NULL;
		// 	- currentVc = 0;
		// 	- idleChannel = 7;
		// Links the master channel with this physical channel through the "this" pointer. 
		// Creates a vector of eight virtual channels. All, except the last one, are empty (they point to NULL).
		// The last one is considered idle and it is created with the following (default) attributes:
		// 	- virtualChannelId = 7;
		// 	- packetSink = NULL;
		// 	- secondHeaderPresent = false;
		// 	- extendedFrameCountSet = false;
		// 	- dataFieldSynchronised = true;
		// 	- initialConf = netProtConf = new NetProtConf;
		// 	- debugOutput = false;
		// 	- sendFrameCount = 0;
		// 	- recFrameCount = 0;
		// 	- recPacketHeaderLength = 0;
		// 	- recPacketLength = 0;
		// 
		// NOTE: A TmMasterChannelError is thrown if:
		// - The Spacecraft ID does not fall within the range of 0 to 1023.
		// - The idle Virtual Channel ID does not fall within the range of 0 to 7.
	} catch (TmMasterChannelError& e) {
		ostringstream error;
		error << "Error creating TmMasterChannel: " << e.what() << endl;
		throw TmPhysicalChannelError(error.str());
	}
	return masterChannel;
}

// Unwraps and analyzes a raw frame for master/virtual channel setting discrepancies and displays the corresponding warnings.
TmChannelWarning TmPhysicalChannel::receiveFrame(vector<uint8_t> rawFrame, TmFrameTimestamp timestamp, TmFrameBitrate bitrate)
{
	TmChannelWarning warning;			// Creates an instance of the TmChannelWarning class.
	try {
		TmTransferFrame frame (frameLength);	// Creates a TM Transfer Frame object to receive the data carried by the raw frame.
		frame.setTimestamp(timestamp);		// Passes the reference timestamp to the frame.
		frame.setBitrate(bitrate);		// Passes the reference bitrate to the frame.

		if (fecfPresent) {						// Checks the FECF Flag and activates it if used in this physical channel.
			frame.activateFecf();
		}
		frame.unwrap(rawFrame);		// Takes the raw frame, reads its fields and flags and stores them in the new frame.
		if (masterChannel) {		// If a master channel has been defined for this physical channel,
			warning += masterChannel->receiveFrame(frame);	// assign the received frame to its corresponding master channel and accumulate any warnings thrown.
		} else {
			// warning message
			warning.setUnconfiguredMC();	// Otherwise throw a warning that no master channel has been configured.
		}
	// Scan for any TM Transfer Frame errors.
	} catch (TmTransferFrameError& e) {		
		warning.addFrameUnwrapError(string(e.what()));
	}
	return warning;		// Returns any warnings found.
}

// Prepares a new TM Transfer Frame to be sent over a master- and virtual channel, if defined.
vector<uint8_t> TmPhysicalChannel::sendFrame(ticp::Timestamp timestamp)
{
	vector<uint8_t> rawFrame;	// Declares a variable to store the raw frame to send.

	if (masterChannel) {	// If a master channel has been defined for this physical channel,
		try {
			TmTransferFrame frame = masterChannel->sendFrame(timestamp);	// Prepares a frame to be sent in this master channel and 
																			// in an available VC (scheduled using RR) with the current timestamp.
			if (frame.getFecfStatus() == fecfPresent) {		// If the frame FECF configuration and
				if (frame.getLength() == frameLength) {		// the total frame lenght match the physical channel configuration,
					rawFrame = frame.wrap();				// The frame is wrapped and we get a new raw frame.
				} else {
					ostringstream error;					// Otherwise we send an error message with wrong frame length.
					error << "Received frame from master channel has wrong frame length. It is ";
					error << dec << frame.getLength() << " and should be ";
					error << dec << frameLength << endl;
					throw TmPhysicalChannelError(error.str());
				}
			} else {	// If the FECF configurations don't match, we send an error message.
				ostringstream error;
				error << "Received frame from master channel has wrong FECF setting.";
				error << " It should be " << dec << fecfPresent << "." << endl;
				throw TmPhysicalChannelError(error.str());
			}
		// Scan for any TM Transfer Frame errors.
		} catch (TmTransferFrameError& e) {
			ostringstream error;
			error << "Error in TmTransferFrame: " << e.what() << endl;
			throw TmPhysicalChannelError(error.str());
		// Scan for any TM Master Chanel errors.
		} catch (TmMasterChannelError& e) {
			ostringstream error;
			error << "Error in TmMasterChannel: " << e.what() << endl;
			throw TmPhysicalChannelError(error.str());
		}
	} else {	// If no master channel has been defined for this physical channel, we throw an error.
		ostringstream error;
		error << "No master channel defined and packet send request received." << endl;
		throw TmPhysicalChannelError(error.str());
	}
	return rawFrame;	// A new TM Transfer Frame is born!
}
