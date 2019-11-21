/**
        Copyright 2013 Institute for Communications and Navigation, TUM

        This file is part of tmtp.

tmtp is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

tmtp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tmtp. If not, see <http://www.gnu.org/licenses/>.
*/
#include "TmMasterChannel.h"
#include "TmPhysicalChannel.h"
#include "TmVirtualChannel.h"
#include "TmTransferFrame.h"
#include "TmOcf.h"
#include "GroundOcfServer.h"
#include "myErrors.h"
#include "TmFrameTimestamp.h"

#include <vector>
#include <sstream>
#include <stdint.h>

using namespace std;

// Constructor of the TmMasterChannel class.
TmMasterChannel::TmMasterChannel(uint16_t scid, TmPhysicalChannel *parent)
{
	if (scid < 1024) {		// Verifies the Spacecraft ID falls within the range of 0 to 1023.
		spacecraftId = scid;
	} else {
		ostringstream error;
		error << "Spacecraft ID out of range (0-1023)." << endl;
		throw TmMasterChannelError(error.str());
	}
	physicalChannel = parent;			// Links this master channel with a physical channel through this pointer.

    ocfPresent = true;					// Will use the Operational Control Field.
    sendFrameCount = 0;					// Resets the sent frame counter.
    recFrameCount = 0;					// Resets the received frame counter.
    secondHeaderPresent = false;		// Will NOT use the TM Secondary Header,
	extendedVcFrameCountUsed = false;	// 	and therefore no extended VC Frame Counter will be used.

	ocfSink = NULL;						// Sets the ocfSink pointer to NULL.
	virtualChannels.assign(8, reinterpret_cast<TmVirtualChannel*>(NULL));	// This master channel will contain eight virtual channels (all point to NULL).
	currentVc = 0;						// Initializes the current Virtual Channel indicator to zero.

	idleChannel = 7;					// Considers the 8th virtual channel as idle.
	try {
		virtualChannels[idleChannel] = new TmVirtualChannel(idleChannel, this);	// Links a virtual channel to this master channel with the following attributes:
			// virtualChannelId = idleChannel;
			// packetSink = NULL;
			// secondHeaderPresent = false;
			// extendedFrameCountSet = false;
			// dataFieldSynchronised = true;
			// initialConf = netProtConf = new NetProtConf;
			// debugOutput = false;
			// sendFrameCount = 0;
			// recFrameCount = 0;
			// recPacketHeaderLength = 0;
			// recPacketLength = 0;
		
	} catch (TmVirtualChannelError& e) {	// If the idle Virtual Channel ID does not fall within the range of 0 to 7,
		ostringstream error;				// an error will be thrown.
		error << "Error creating TmVirtualChannel " << e.getVcid() << ": " << e.what() << endl;
		throw TmMasterChannelError(error.str());
	}
}

// Destructor of the TmMasterChannel class.
TmMasterChannel::~TmMasterChannel()
{
	for (int i = 0; i < 8; i++) {
		delete virtualChannels[i];	// Removes all eigth of the generated virtual channels from memory.
	}
}

// Retrieves the Spacecraft ID.
uint16_t TmMasterChannel::getSpacecraftId()
{
	return spacecraftId;
}

// Retrieves the sent frame counter.
uint16_t TmMasterChannel::getSendFrameCount()
{
	return sendFrameCount;
}

// Retrieves the received frame counter.
uint16_t TmMasterChannel::getRecFrameCount()
{
	return recFrameCount;
}

// Retrieves the total frame length as configured in the physical channel.
uint16_t TmMasterChannel::getFrameLength()
{
	return physicalChannel->getFrameLength();
}

// Retrieves the FECF flag as configured in the physical channel.
bool TmMasterChannel::getFecfStatus()
{
	return physicalChannel->getFecfStatus();
}

// Sets the OCF Flag to TRUE.
void TmMasterChannel::activateOcf()
{
	ocfPresent = true;
}

// Sets the OCF Flag to FALSE.
void TmMasterChannel::deactivateOcf()
{
	ocfPresent = false;
}

// Retrieves the value of the OCF Flag.
bool TmMasterChannel::getOcfStatus()
{
	return ocfPresent;
}

// Places an OCF message in an output Queue for transmission, iff the OCF Queue has not reached its limit.
void TmMasterChannel::sendOcf(TmOcf ocf)
{
	if (sendOcfFifo.size() < sendOcfBufferSize) {	// If the OCF queue has not reached the established limit,
		sendOcfFifo.push(ocf);						// the OCF message is placed in the queue.
	} else {
		ostringstream error;						// Otherwise, throws an error.
		error << "Send OCF buffer overflow." << endl;
		throw TmMasterChannelError(error.str());
	}
	sendOcfFifo.push(ocf);		// CHECK THIS: Are we queuing the OCF message twice?
}

// Retrieves the 1st OCF message in the input queue, if any.
TmOcf TmMasterChannel::receiveOcf()
{
	TmOcf ocf;							// Creates an OCF instance to host the received OCF message.
	if (this->ocfAvailable()) {			// If the OCF input queue is NOT empty,
		ocf = recOcfFifo.front();		// store the 1st OCF message in the queue in the recently created OCF instance.
		recOcfFifo.pop();
	} else {
		ostringstream error;			// Otherwise, throws an error.
		error << "No OCF available." << endl;
		throw TmMasterChannelError(error.str());
	}
	return ocf;			// Returns the retrieved OCF message.
}

// Returns TRUE if there is any OCF message in the OCF input queue.
bool TmMasterChannel::ocfAvailable()
{
	return !recOcfFifo.empty();
}

// Establishes an whole channel as idle (default is 7).
void TmMasterChannel::setIdleChannel(uint16_t channel)
{
	if (channel < 8) {				// Verifies if the requested channel to be idle falls within the allowed range (0-7).
		delete virtualChannels[idleChannel];	// Frees any memory allocated to the current idle channel.
		idleChannel = channel;					// Establishes the requested channel as the new idle channel.
		delete virtualChannels[idleChannel];	// Frees any memory allocated to the new idle channel.
		try {
			virtualChannels[idleChannel] = new TmVirtualChannel(idleChannel, this);	// Checks for any errors that might pop up.
		} catch (TmVirtualChannelError& e) {
			ostringstream error;				// And throws the corresponding error messages.
			error << "Error creating TmVirtualChannel " << e.getVcid() << ": " << e.what() << endl;
			throw TmMasterChannelError(error.str());
		}
	} else {
		ostringstream error;
		error << "Virtual Channel ID for idle channel out of range (0-7)." << endl;
		throw TmMasterChannelError(error.str());
	}
}

// Retrieves the current idle channel.
uint16_t TmMasterChannel::getIdleChannel()
{
	return idleChannel;
}

// Extracts the contents of the idle channel.
TmVirtualChannel* TmMasterChannel::getIdleChannelObject()
{
	return virtualChannels[idleChannel];
}

// Checks each virtual channel for available frames to send.
bool TmMasterChannel::frameAvailable()
{
	bool a = false;
	for (int i = 0; i < 8; i++) {
		if (virtualChannels[i]) {
			a |= virtualChannels[i]->frameAvailable();
		}
	}
	return a;
}

// Verifies if a received frame has the correct settings for this master channel.
TmChannelWarning TmMasterChannel::receiveFrame(TmTransferFrame frame)
{
	TmChannelWarning warning;		// Creates an instance of TmChannelWarning to receive any warnings/errors occured.

	// check scid
	if (frame.getSpacecraftId() != spacecraftId) {
		// warning message
		warning.setWrongScid();
	} else {
		// check frame count
		if (recFrameCount == frame.getMasterChannelFrameCount()) {
			recFrameCount = (recFrameCount+1) % 256;	// Increases the counter and performs a modulo 256
		} else {
			// warning message
			warning.addMCLostFramesCount((frame.getMasterChannelFrameCount() - recFrameCount + 256) % 256);
			recFrameCount = (frame.getMasterChannelFrameCount()+1) % 256;	// Rectifies the received frame counter.
		}
		// check OCF flag
		if (ocfPresent != frame.getOcfStatus()) {
			// warning message
			warning.setWrongOcfFlag();
		}
		// get TmOcf
		if (ocfPresent && frame.getOcfStatus()) {
			if (recOcfFifo.size() < recOcfBufferSize) {	// If the received OCF queue in this phy channel has not reached its limit,
				recOcfFifo.push(frame.getOcf());		// place the OCF of the received frame into the input queue and
				warning += this->signalNewOcf();		// send a warning that a new OCF was queued.
			} else {
				// warning message
				warning.setRecOcfBufferOverflow();		// Otherwise send a buffer overflow warning.
			}
		}
		uint16_t vcid = frame.getVirtualChannelId();	// Extracts the frame's Virtual Channel ID and...
		if (virtualChannels[vcid]) {					// ... If such a VC is configured (i. e. it exists):
			 warning += virtualChannels[vcid]->receiveFrame(frame);		// The frame is sent/assigned to that VC.
		} else {
			// warning message
			warning.setUnconfiguredVC();
		}
	}
	return warning;
}

// Prepares a frame according to the physical channel settings to be sent over a virtual channel and appends a timestamp.
TmTransferFrame TmMasterChannel::sendFrame(TmFrameTimestamp timestamp)
{
	TmTransferFrame frame(7);	// Creates a TM Transfer Frame with the minimum size.
	try {
		frame = TmTransferFrame(physicalChannel->getFrameLength()); // Redimensions the frame to match the length configured in the physical channel.
		uint16_t vcid;
		uint16_t i;

		// Round Robin scheduling
		for (i = 0; i < 8; i++) {
			vcid = (currentVc + i) % 8;		// Cycles through all the VCs (modulo 8 is used start over with the 1st element after being in the last)
			if (virtualChannels[vcid] && (vcid != idleChannel)) {	// If the current VC is not the idle channel (default = 7),
				if (virtualChannels[vcid]->frameAvailable()) {			// and if the that VC has a frame to send,
					frame = virtualChannels[vcid]->sendFrame(timestamp);	// Configures and populates a frame to be sent in this virtual channel.
					break;					// Exits the Round Robin scheduling.
				}
			}
		}
		if (i == 8) {	// If after the Round Robin we ended up in the last position, 
			frame = virtualChannels[idleChannel]->sendFrame(timestamp);	// Uses the idle channel to send an idle frame.
		}
		currentVc = (vcid + 1) % 8;		// Next time the RR is executed, the initial position will be increased by one.

		// set master channel settings
		frame.setSpacecraftId(spacecraftId);
		frame.setMasterChannelFrameCount(sendFrameCount);

		if (frame.getOcfStatus() == ocfPresent) {	// Verifies the OCF Flags match
			if (ocfPresent) {						// and if using the OCF:
				// add ocf
				TmOcf ocf;					// Creates an OCF instance.
				if (!sendOcfFifo.empty()) {		// If the OCF output queue is not empty
					ocf = sendOcfFifo.front();		// takes the 1st OCF message in the queue
					sendOcfFifo.pop();				// and pops that entry out of the queue.
				}	
				else {						// If the OCF output queue IS empty...
					//TODO: iocf what is the correct idle Ocf?
					ocf.setReportType(TmOcf::Type2FutureReserved);
				}
				frame.setOcf(ocf);	// The requested OCF is inserted in the frame.
			}
		} else {		// If the OCf flags don't match, an error is thrown.
			ostringstream error;
			error << "Received frame from virtual channel has wrong OCF setting." << endl;
			throw TmMasterChannelError(error.str());
		}
		sendFrameCount = (sendFrameCount+1) % 256;	// The sent frames counter is increased.
	} catch (TmTransferFrameError& e) {
		ostringstream error;
		error << "Error in TmTransferFrame: " << e.what() << endl;
		throw TmMasterChannelError(error.str());
	} catch (TmVirtualChannelError& e) {
		ostringstream error;
		error << "Error in TmVirtualChannel " << e.getVcid() << ": " << e.what() << endl;
		throw TmMasterChannelError(error.str());
	}
	return frame;	// A new frame is born and ready to be sent.
}

// Establishes the sink where OCF messages are received (GroundOcfServer instance).
void TmMasterChannel::connectOcfSink(GroundOcfServer *sink)
{
	ocfSink = sink;
}

// Sets the OCF sink pointer to NULL.
void TmMasterChannel::disconnectOcfSink()
{
	ocfSink = NULL;
}

// Creates a new VC instance with default values and places a new pointer to it in the VC vector.
TmVirtualChannel* TmMasterChannel::createTmVirtualChannel(uint16_t vcid)
{
	TmVirtualChannel *newVc = NULL;	// Creates a new virtual channel pointer.

	if (vcid < 8) {					// If the specified VC ID is within boundaries,
		if (vcid != idleChannel) {			// and if is not the idle channel (default idle channel is 7)
			try {
				newVc = new TmVirtualChannel(vcid, this);	// Creates a new VC linked to this master channel.
			} catch (TmVirtualChannelError& e) {
				ostringstream error;
				error << "Error creating TmVirtualChannel " << e.getVcid() << ": " << e.what() << endl;
				throw TmMasterChannelError(error.str());
			}
			if (virtualChannels[vcid]) {	// If there is already a VC object defined for the specified VC ID,
				delete virtualChannels[vcid];	// it will be removed from memory.
			}
			virtualChannels[vcid] = newVc;	// Now we place the newly created VC instance's pointer in the VC vector.
		} else {					// If the specified VC ID is the idle channel.
			ostringstream error;
			error << "Virtual channel " << dec << vcid << " is already configured as idle channel." << endl;
			throw TmMasterChannelError(error.str());
		}
	} else {
		ostringstream error;
		error << "Virtual Channel ID out of range (0-7)." << endl;
		throw TmMasterChannelError(error.str());
	}
	return newVc;
}

// Removes the specified virtual channel entry from memory and sets its contents to point to NULL.
void TmMasterChannel::deleteTmVirtualChannel(uint16_t vcid)
{
	delete virtualChannels[vcid];
	virtualChannels[vcid] = NULL;
}

// Retrieves the 1st OCF message in the OCF sink reception queue (if any) and displays the corresponding debug messages (optional).
TmChannelWarning TmMasterChannel::signalNewOcf()
{
	TmChannelWarning warning;		// Creates a TmChannelWarning instance to receive warnings.
	if (ocfSink) {					// If an OCF sink is defined (instance of GroundOcfServer)
		try {
			ocfSink->signalNewOcf();	// Retrieves the 1st OCF message in the reception queue and optionally displays the corresponding debug messages.
		} catch (GroundOcfServerError& e) {
			ostringstream error;		// Any error messages will be appended to the string of free messages.
			error << "Error in GroundOcfServer connected to MC: " << e.what();
			warning.appendFreeMessage(error.str());
		}
	} else {
									// A warning message is sent if no OCF sink has been defined.
		warning.setNoOcfSinkSpecified();
	}
	return warning;
}
