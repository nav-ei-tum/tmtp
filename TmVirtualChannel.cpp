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
#include "TmVirtualChannel.h"
#include "TmTransferFrame.h"
#include "TmMasterChannel.h"
#include "TmFrameTimestamp.h"
#include "TmFrameBitrate.h"
#include "GroundPacketServer.h"
#include "NetProtConf.h"
#include "myErrors.h"
#include "TmFrameTimestamp.h"

#include <boost/function.hpp>

#include <iterator>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdint.h>

using namespace std;

// Constructor of the TmVirtualChannel class.
TmVirtualChannel::TmVirtualChannel(uint16_t id, TmMasterChannel *parent)
{
	if (id < 8) {		// Verifies the Virtual Channel ID falls within the range of 0 to 7.
		virtualChannelId = id;
	} else {
		ostringstream error;
		error << "Virtual Channel ID out of range (0-7)." << endl;
		throw TmVirtualChannelError(virtualChannelId, error.str());
	}
	masterChannel = parent;		// Links this virtual channel with a master channel through this pointer.

	packetSink = NULL;				// Initializes the ground packet server pointer to NULL.
	secondHeaderPresent = false;	// Will NOT use the TM Secondary Header,
	extendedFrameCountSet = false;	//	and therefore no extended VC Frame Counter will be used.
	dataFieldSynchronised = true;	// The packet format in the TM Data Field will be Byte-synchronized, forward-ordered.
	initialConf = netProtConf = new NetProtConf;	// The initial configuration considers all packets as idle.
	debugOutput = false;			// Does not include debug output in the warning messages.

	// Initializes all the counters to zero
	sendFrameCount = 0;
	recFrameCount = 0;
	recPacketHeaderLength = 0;
	recPacketLength = 0;
}

// Destructor of the TmVirtualChannel class.
TmVirtualChannel::~TmVirtualChannel()
{
	delete initialConf;	// Removes the generated initial config object from memory.
}

// Retrieves the Virtual Channel ID.
uint16_t TmVirtualChannel::getVirtualChannelId()
{
	return virtualChannelId;
}

// Retrieves the sent frame counter.
uint64_t TmVirtualChannel::getSendFrameCount()
{
	return sendFrameCount;
}

// Retrieves the received frame counter.
uint64_t TmVirtualChannel::getRecFrameCount()
{
	return recFrameCount;
}

// Sets the Secondary Header and the extended frame counter flags to TRUE.
void TmVirtualChannel::activateExtendedFrameCount()
{
	secondHeaderPresent = true;
	extendedFrameCountSet  = true;
}

// Sets the Secondary Header and the extended frame counter flags to FALSE.
void TmVirtualChannel::deactivateExtendedFrameCount()
{
	secondHeaderPresent = false;
	extendedFrameCountSet = false;
}

// Retrieves the value of the extended frame counter flag.
bool TmVirtualChannel::getExtendedFrameCountStatus()
{
	return extendedFrameCountSet;
}

// Sets the direct Data Field access flag to TRUE.
void TmVirtualChannel::activateDirectDataFieldAccess()
{
	directDataFieldAccess = true;
}

// Sets the direct Data Field access flag to FALSE.
void TmVirtualChannel::deactivateDirectDataFieldAccess()
{
	directDataFieldAccess = false;
}

// Retrieves the value of the direct Data Field access flag.
bool TmVirtualChannel::getDirectDataFieldAccessStatus()
{
	return directDataFieldAccess;
}

// Places a packet in the output queue.
void TmVirtualChannel::sendPacket(vector<uint8_t> packet)
{
	if (sendFifo.size() < sendPacketBufferSize) {	// If the output queue has not reached its limit,
		if (sendFifo.empty()) {						// and if the the output queue is empty,
			sendFifo.push(packet);						// place the packet in the output queue and
			sendPointer = sendFifo.front().begin();		// update the iterator position to the oldest element in the queue, 
															// and specifically, to the 1st element of the vector in that position.
		} else {
			sendFifo.push(packet);					// If the queue is not empty, just put packet in the queue and don't touch the iterator.
		}
	} else {
		ostringstream error;						// If the output queue has reached its limit, throw an error.
		error << "Packet buffer overflow." << endl;
		throw TmVirtualChannelError(virtualChannelId, error.str());
	}
}

// Retrieves a packet with its timestamp from the input queue.
TimeTaggedPacket TmVirtualChannel::receivePacket()
{
//	vector<uint8_t> packet;
	TimeTaggedPacket packetAndTimestamp;
	
	if (this->packetAvailable()) {				// If there is any packet waiting in the input queue.
		packetAndTimestamp = recFifo.front();	// The 1st element in the input queue will be stored
		recFifo.pop();							// and popped out of the queue.
		
	} else {
		ostringstream error;			// Otherwise, an error is thrown if there are no available packets.
		error << "No packet available." << endl;
		throw TmVirtualChannelError(virtualChannelId, error.str());
	}
	return packetAndTimestamp;
}

// Indicates whether there are any frames available to send.
bool TmVirtualChannel::frameAvailable()
{
	if (directDataFieldAccess) {	// Enabled Direct Data Field Access means there are frames to send (perhaps because navigation data is constantly sent).
		return true;
	} else {
		return !sendFifo.empty();	// A non-empty output queue means the same.
	}
}

// Indicates whether the input packet queue has an available packet.
bool TmVirtualChannel::packetAvailable()
{
	return !recFifo.empty();
}

// Reads the Data Field of a received frame and extracts the packets it contains (must be unwrapped first).
TmChannelWarning TmVirtualChannel::receiveFrame(TmTransferFrame frame)
{
	// Retrieves the Data Field from the received frame, extracts its data and initializes recPointer to its 1st position.
	vector<uint8_t> data = frame.getDataField();
	vector<uint8_t>::iterator recPointer = data.begin();
	vector<uint8_t>::iterator firstHeaderPointer;
	//bool firstHeaderAlreadyMatched = false; /* not used at the moment */
	
	TmFrameTimestamp frameTimestamp = frame.getTimestamp();	// Extracts the reference timestamp from the frame.
	TmFrameBitrate frameBitrate = frame.getBitrate();		// Extracts the reference bitrate from the frame.

	double rawPacketTimestamp = 0.0;	// Stores the estimated timestamp of each packet before being separated into seconds and fractions.
	TmFrameTimestamp packetTimestamp;	// Temorarily houses the estimated timestamp of each packet.
	TimeTaggedPacket packetAndTimestamp;	// Stores a packet and a timestamp together in a single structure before sending it to the input queue.*/
	
	TmChannelWarning warning;

	// Check for frame setting consistency (compares the values of the received frame against the VC settings.)
	if (frame.getVirtualChannelId() != virtualChannelId) {
		// warning message
		warning.setWrongVcid();
	} else if (frame.getSecondHeaderStatus() != secondHeaderPresent) {
		// warning message
		warning.setWrongSecondHeaderFlag();
	} else if (frame.getDataFieldSynchronisationStatus() != dataFieldSynchronised) {
		// warning message
		warning.setWrongSynchronisationFlag();
	} else {	
	
	// If the consistency check is all hunky dory:
		if (extendedFrameCountSet) {
			try {
				frame.activateExtendedVcFrameCount();	// Reads the SH Data Field to retrieve the 3-Bytes long counter extension.
			} catch (TmTransferFrameError& e) {
				// warning message
				warning.addFrameUnwrapError(string(e.what()));
			}
		}
		
		// The received frame contents are displayed.
		// debug frame output
		if (debugOutput) {
			cout << "Received " << flush;
			frame.debugOutput();
		}
		
		// check frame count
		if (recFrameCount == frame.getVirtualChannelFrameCount()) {
			if (extendedFrameCountSet) {
				recFrameCount = (recFrameCount+1) % ((uint64_t)1<<32); // ((uint64_t)1<<32) = (64-bit wide unsigned int) 2^32
			} else {
				recFrameCount = (recFrameCount+1) % 256;
			}
		} else {
			// discard current packet
			recPacket.clear();
			recPacketHeaderLength = 0;
			recPacketLength = 0;
			
			// warning message
			if (extendedFrameCountSet) {
				warning.addVCLostFramesCount((frame.getVirtualChannelFrameCount()
					- recFrameCount + ((uint64_t)1<<32)) % ((uint64_t)1<<32)); // ((uint64_t)1<<32) = (64-bit wide unsigned int) 2^32
			} else {
				warning.addVCLostFramesCount((frame.getVirtualChannelFrameCount()
					- recFrameCount + 256) % 256);
			}
			
			// Rectify the frame counter.
			if (extendedFrameCountSet) {
				recFrameCount = (frame.getVirtualChannelFrameCount()+1) % ((uint64_t)1<<32); // ((uint64_t)1<<32) = (64-bit wide unsigned int) 2^32
			} else {
				recFrameCount = (frame.getVirtualChannelFrameCount()+1) % 256;
			}
		}

		// Frame unwrapped:				OK
		// Frame consistency check :	OK
		// VC Counters updated :		OK
		// Now we extract the data..
		
		if (frame.getFirstHeaderPointer() != TmTransferFrame::fhpOnlyIdleData) {
			/* handle direct data field access if configured */
			if (directDataFieldAccess) {
				if (directRecvDataFieldAccessFunction) {
					// TODO timestamp processing
					directRecvDataFieldAccessFunction(data,TmFrameTimestamp());
				} else {
					throw TmVirtualChannelError(virtualChannelId,
							"Direct data field access configured but corresponding receive function "
							"pointer not connected.");
					}
			} else { /* normal packet mode */
				// 1st check if the FHP indicates NO packet begins in the TF Data Field. 
				// This happens if a long packet spans across multiple frames.
				if (frame.getFirstHeaderPointer() == TmTransferFrame::fhpNoFirstHeader) {
					firstHeaderPointer = data.end();
				} else {	// If at least one packet begins in the Data Field...
					// ... It might not necessarily begin on the very 1st Byte on the Data Field.
					// We need to adjust firstHeaderPointer to point where a packet begins:
					firstHeaderPointer = data.begin() + frame.getFirstHeaderPointer();
					}
				
				// Now we scan the whole Data Field:
				while (recPointer < data.end()) {
					// recPacket is a "pre-buffer" in which we store the Bytes (or pieces) of a packet.
					// When a packet header is detected in the Data Field, each Byte is copied, one-by-one, into the pre-buffer.
					// Once the header of a new packet is detected, the contents of recPacket are assembled as a whole packet and sent to the input queue.
					// recPacket is then cleared and populated with the Bytes of the next packet.
					// This pre-buffer mechanism is especially useful if a packet spans across several frames.
					
					// If we are receiving the very first Byte of a new packet:
					if (recPacket.size() == 0) {
						if (recPointer < firstHeaderPointer) {	// Verify recPointer points to the start of a packet.
							recPointer = firstHeaderPointer;	// Otherwise adjust the pointer.
							//firstHeaderAlreadyMatched = true;
							warning.setPacketResynced();		// Let the application know that the pointer was adjusted:
																// FHP did not point to the 1st Byte of the Data Field,
																// probably because the last piece of a packet in the previous frame 
																// was left at the beginning of the current Data Field.

						} else {	// ... But if recPointer actually points to the start of a packet:
							if (netProtConf->isIdlePacket(*recPointer)) {	// Check if, according to the packet protocol config, we have an idle packet.
								recPointer++;								// In which case, we simply ignore the packet.

							} else {	// If we are dealing with a packet with actual data:
								recPacketHeaderLength = netProtConf->getPacketHeaderLength(*recPointer);	// Extract the packet header length.
								recPacket.push_back(*recPointer);			// Place current Byte in the pre-buffer.
								
								// The following procedure is to be done on the first Byte of each packet:
								if (frameTimestamp.isValid() && frameBitrate.isValid()) {	// If the timestamp *and* bitrate stored in the frame 
																							// contain actual data:
									// Take the Primary_Header_Length + Secondary_Header_Length + Data_Field_pos._of _1st_packet_Byte 
									// and divide that by the reference bitrate stored in the frame to get a "raw packet timestamp".
									int position = distance(data.begin(),recPointer);
									rawPacketTimestamp = ((6 + frame.getSecondHeaderLength() + position) * 8) / frameBitrate.getBitrate();
									
									// Now we put the seconds and fractions in their respective places within the packet timestamp object:
									packetTimestamp.setSeconds(frameTimestamp.getSeconds() + static_cast<int>(rawPacketTimestamp));
									packetTimestamp.setFractions(rawPacketTimestamp - static_cast<int>(rawPacketTimestamp));
									// And we put the new timestamp as well as the reference bitrate in the joint data structure, waiting for the complete packet to be assembled.
									packetAndTimestamp.timestamp = packetTimestamp;
									packetAndTimestamp.bitrate = frameBitrate;
								}
								recPointer++;	// ... On to the next Byte.
								}
							}
						
					// If other pieces of this packet have been received:
					} else {
						if (recPacket.size() < recPacketHeaderLength) { // If current Byte is (still) within the packet header:
							recPacket.push_back(*recPointer);			// Place current Byte in the pre-buffer.
							recPointer++;								// And jump to the next Byte.
																		// ... But before reading the next Byte:
							if (recPacket.size() == recPacketHeaderLength) { // Check if the previous Byte was the last one (i.e. header completely read):
								recPacketLength = netProtConf->extractPacketLength(recPacket);	// Extract the Packet Length.
							}
							
						} else {										// If the packet header was completely read:
							if (recPointer == firstHeaderPointer) { 	// Check if the pointer SOMEHOW got to the FHP position:
								recPacket.clear();						// Discard current packet (clear the pre-buffer).
								recPacketHeaderLength = 0;
								recPacketLength = 0;
								//firstHeaderAlreadyMatched = true;
								packetAndTimestamp.timestamp = packetTimestamp = TmFrameTimestamp();	// The Timestamps are discarded. 
								rawPacketTimestamp = 0.0;
								packetAndTimestamp.bitrate = TmFrameBitrate();							// The stored bitrate is also discarded. 
								warning.setPacketResynced();			// ... And a warning message is sent.

							} else {									// ... But if we are still on track and nothing funny has happened:
								recPacket.push_back(*recPointer);		// Place current Byte in the pre-buffer.
								recPointer++;							// And jump to the next Byte.
								// ... Now the magic:
								if (recPacket.size() == recPacketLength) {		// If the packet has been completely read:
									if (recFifo.size() < recPacketBufferSize) { // Check if the input queue can store one more packet.
										packetAndTimestamp.data = recPacket;	// Store the assembled packet in the joint data structure.
										recFifo.push(packetAndTimestamp);		// Place the joint data structure in the input queue.
										recPacket.clear();						// Discard current packet in the pre-buffer.
										recPacketHeaderLength = 0;
										recPacketLength = 0;
										packetTimestamp = TmFrameTimestamp();	// The information stored in the Timestamp instance is discarded. 
										rawPacketTimestamp = 0.0;
										warning += this->signalNewPacket();		// And finally, tell the application we have a new packet!

									} else {									// If we have reached the maximum amount of inbound packets, 
										warning.setRecPacketBufferOverflow();	// throw a warning about the buffer overflow.
										}
								}
								}
							}
						}
				}
				}
		}
	}
	return warning;
}

// Creates a new TM frame, adjusts its settings and populates its Data Field.
TmTransferFrame TmVirtualChannel::sendFrame(TmFrameTimestamp timestamp)
{
	vector<uint8_t> data;				// Vector holding data for preparation to be encapsulated in the frame.
	uint64_t availableDataLength;		// Space available in the Data Field to host a packet (in Bytes).
	uint16_t neededDataLength;			// Amount of the available Data Field space needed to host a packet (in Bytes).
	uint16_t frameDataLength;			// Total space available in the Data Field (in Bytes).
	uint16_t firstHeaderPointer = TmTransferFrame::fhpNoFirstHeader;	// Location of the 1st Byte of the 1st packet in the Data Field.
	TmTransferFrame frame(7);			// TM Frame initialized with the minimum size (seven Bytes).

	try {
		// First thing to do: Adjust the new frame's properties and settings to match the master- and virtual channel:
		
		frame = TmTransferFrame(masterChannel->getFrameLength());	// Redimensions the frame using the settings of the master channel.
		frame.setVirtualChannelId(virtualChannelId);				// Gets the frame VC ID from the current VC.
		if (masterChannel->getOcfStatus()) {						
			frame.activateOcf();									// If specified in the master channel settings, sets the OCF Flag to TRUE.
		}
		if (masterChannel->getFecfStatus()) {
			frame.activateFecf();									// If specified in the master channel settings, sets the FECF Flag to TRUE.
		}
		if (secondHeaderPresent) {
			frame.activateSecondHeader();							// If specified in the VC settings, sets the Secondary Header Flag to TRUE.
		}
		if (extendedFrameCountSet) {
			frame.activateExtendedVcFrameCount();					// If specified in the VC settings, uses an extended frame counter.
		}
		if (dataFieldSynchronised) {
			frame.activateDataFieldSynchronisation();				// If specified in the VC settings, sets the Synchronization Flag to TRUE.
		}
		frame.setVirtualChannelFrameCount(sendFrameCount);			// Gets the current transmitted frame counter.
		
		// Important thing to do: calculate the Data Field length.
		frameDataLength = frame.getDataFieldLength();

		// If using the Direct Data Field Access (DDFA) method to insert "raw data" in the Data Field:
		if (directDataFieldAccess) {
			firstHeaderPointer = 0;						// Start at the first Byte of the Data Field.
			if (directSendDataFieldAccessFunction) {	// If a Tx DDFA wrapper function has been defined:

				// Use that function to process the raw data and store it in the data vector to be encapsulated
				data = directSendDataFieldAccessFunction(frameDataLength, timestamp);

				if (data.size() != frameDataLength)
					throw TmVirtualChannelError(virtualChannelId,
							"Direct data field access function return data field of wrong size.");
			} else {
				throw TmVirtualChannelError(virtualChannelId,
						"Direct data field access configured but corresponding send function pointer "
						"not connected.");
			}
			
		// If using the "normal packet mode" to insert packets in the Data Field:
		} else {
			while (data.size() < frameDataLength) {		// While the current data still fits in the Data Field:
				neededDataLength = frameDataLength - data.size();	// Compute the amount of the available Data Field space to store data.
				
				// If there are packets waiting to be transmitted:
				if (!sendFifo.empty()) {
					availableDataLength = sendFifo.front().end() - sendPointer;		// Calculate the length of the 1st (not yet sent) packet in the output queue.
					
					// If the packet does NOT fit in the Data Field:
					if (availableDataLength > neededDataLength) {
						// Check whether this packet is the very first in the queue and that the FHP is set to the default:
						if ((sendPointer == sendFifo.front().begin())
								&& (firstHeaderPointer == TmTransferFrame::fhpNoFirstHeader)) {
							firstHeaderPointer = data.size();	// In this case the FHP shall point to the beginning of the data.
						}
						data.insert(data.end(), sendPointer, sendPointer + neededDataLength);	// Insert as many Bytes of output queue into the end of 
																								// the data vector as they can fit in the Data Field.
						sendPointer += neededDataLength;										// Update the output-queue transmitted-bytes-pointer.
						// This sendPointer will help us to place oversized packets in the data vector which could not fit in a single frame.
						// If only the half of a packet could be placed in the data vector, the sendPointer will show the location where the packet was cut.
						// This way, next time a data vector is filled, it will begin at the position of sendPointer, thus inserting the rest of the packet.
						// More importantly, if this is the case, the FHP will not be updated! It will only point to the location where a new packet BEGINS.
					} 
					
					// If the packet fits in the Data Field:
					else {
						// Checks whether this packet is the very first in the queue and that the FHP is set to the default:
						if ((sendPointer == sendFifo.front().begin())
								&& (firstHeaderPointer == TmTransferFrame::fhpNoFirstHeader)) {
							firstHeaderPointer = data.size();	// In this case the FHP shall point to the beginning of the data.
						}
						data.insert(data.end(), sendPointer, sendFifo.front().end());	// Insert the whole contents of one element of the output queue into 
																						// the data vector (i.e. one whole packet).
						sendFifo.pop();													// Then, delete current packet in the queue as it will be completely sent.
						if (!sendFifo.empty()) {
							sendPointer = sendFifo.front().begin();			// Update the output-queue transmitted-bytes-pointer accordingly if the queue is empty.
						}
					}
				} 
				
				// If there are no more packets to send:
				else {
					if (firstHeaderPointer == TmTransferFrame::fhpNoFirstHeader) {
						if (data.size() != 0) {		// In case a chunk of a big packet remained in the data vector,
							firstHeaderPointer = data.size();	// place the FHP to the end of the data...
						} else {
						// If there was no data at all to send,
							firstHeaderPointer = TmTransferFrame::fhpOnlyIdleData; // Set the FHP to the predefined pattern to indicate idle contents...
						}
					}
					// ... And put an idle packet at the end of the data vector. 
					data.push_back(netProtConf->genIdlePacket());
				}
			}
		}

		// After the data vector has been locked and loaded:
		frame.setFirstHeaderPointer(firstHeaderPointer);	// Set the frame FHP as indicated by the previous process.
		frame.setDataField(data);							// And insert the holy data vector into the Data Field.

		if (extendedFrameCountSet) {						// Update the Tx Frame Counter
			sendFrameCount = (sendFrameCount+1) % ((uint64_t)1<<32); // ((uint64_t)1<<32) = (64-bit wide unsigned int) 2^32
		} else {
			sendFrameCount = (sendFrameCount+1) % 256;
		}
		
		frame.debugOutput();	// Dissects the frame into its components and displays them.

	} catch (TmTransferFrameError& e) {						// Catch any errors.
		ostringstream error;
		error << "Error in TmTransferFrame: " << e.what() << endl;
		throw TmVirtualChannelError(virtualChannelId, error.str());
	}

	return frame;	// A new frame has been born under this virtual channel!
}

// Establishes the sink where OCF messages are received (GroundPacketServer instance).
void TmVirtualChannel::connectPacketSink(GroundPacketServer *sink)
{
	packetSink = sink;
}

// Sets the packet sink pointer to NULL.
void TmVirtualChannel::disconnectPacketSink()
{
	packetSink = NULL;
}

// Establishes the packet configuration for this channel.
void TmVirtualChannel::setNetProtConf(NetProtConf *conf)
{
	netProtConf = conf;
}

// Specifies the function used to directly access the Data Field in order to transmit.
void TmVirtualChannel::connectDirectSendDataFieldAccessFunction(
		boost::function<vector<uint8_t>(uint16_t, TmFrameTimestamp)> function)
{
	directSendDataFieldAccessFunction = function;
}

// Specifies the function used to directly access the Data Field of a received frame.
void TmVirtualChannel::connectDirectRecvDataFieldAccessFunction(
		boost::function<void(vector<uint8_t> const&, TmFrameTimestamp)> function)
{
	directRecvDataFieldAccessFunction = function;
}

// Sets the debug output flag to TRUE.
void TmVirtualChannel::activateDebugOutput()
{
	debugOutput = true;
}

// Sets the debug output flag to TRUE.
void TmVirtualChannel::deactivateDebugOutput()
{
	debugOutput = false;
}

// Retrieves the 1st packet in the packet sink reception queue (if any) and displays the corresponding debug messages.
TmChannelWarning TmVirtualChannel::signalNewPacket()
{
	TmChannelWarning warning;		// Creates a TmChannelWarning instance to receive warnings.
	if (packetSink) {				// If a packet sink is defined (instance of GroundPacketServer)
		try {
			packetSink->signalNewPacket();	// Retrieves the 1st packet in the reception queue and displays the corresponding debug messages.
		} catch (GroundPacketServerError& e) {
			ostringstream error;			// Any error messages will be appended to the string of free messages.
			error << "Error in GroundPacketServer connected to VC " << virtualChannelId;
			error << ": " << e.what();
			warning.appendFreeMessage(error.str());
		}
	} else {
		// warning message			// A warning message is sent if no packet sink has been defined.
		warning.setNoPacketSinkSpecified();
	}
	return warning;
}
