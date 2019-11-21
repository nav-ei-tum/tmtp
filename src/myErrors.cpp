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
#include "myErrors.h"
#include <sstream>
#include <string>
#include <stdint.h>

using namespace std;

// Constructor for the Channel Warnings class.
TmChannelWarning::TmChannelWarning()
{
	frameUnwrapError.clear();
	lostMCFrames = 0;
	lostVCFrames = 0;
	packetResync = false;
	noPacketSinkSpecified = false;
	noOcfSinkSpecified = false;
	unconfiguredVC = false;
	unconfiguredMC = false;
	recPacketBufferOverflow = false;
	recOcfBufferOverflow = false;
	wrongOcfFlag = false;
	wrongScid = false;
	wrongVcid = false;
	wrongSecondHeaderFlag = false;
	wrongSynchronisationFlag = false;
	freeMessage.clear();
}

// Overloading operator function for "+=" accumulates error messages.
TmChannelWarning & TmChannelWarning::operator+=(const TmChannelWarning &rhs)
{
	// String and integer errors are accumulated with "+=".
	// Boolean errors are OR'd - Each type of error needs to happen just once to be marked as present. 
	frameUnwrapError += rhs.frameUnwrapError;
	lostMCFrames += rhs.lostMCFrames;
	lostVCFrames += rhs.lostVCFrames;
	
	packetResync |= rhs.packetResync;
	noPacketSinkSpecified |= rhs.noPacketSinkSpecified;
	noOcfSinkSpecified |= rhs.noOcfSinkSpecified;
	unconfiguredVC |= rhs.unconfiguredVC;
	unconfiguredMC |= rhs.unconfiguredMC;
	recPacketBufferOverflow |= rhs.recPacketBufferOverflow;
	recOcfBufferOverflow |= rhs.recOcfBufferOverflow;
	wrongOcfFlag |= rhs.wrongOcfFlag;
	wrongScid |= rhs.wrongScid;
	wrongVcid |= rhs.wrongVcid;
	wrongSecondHeaderFlag |= rhs.wrongSecondHeaderFlag;
	wrongSynchronisationFlag |= rhs.wrongSynchronisationFlag;
	
	freeMessage += rhs.freeMessage;
	return *this;
}

// Removes newlines, replaces them with blank spaces, appends current msg into frameUnwrapError and adds a semicolon at its end.
void TmChannelWarning::addFrameUnwrapError(string msg)
{
	// delete all newlines
	size_t found=msg.find_first_of("\n");	// Searches for the position of the first "\n" found in msg.
	while (found!=string::npos) {			// While the current position has not become the last position.
		msg[found]=' ';						// Replace the character in current position with a blank space.
		found=msg.find_first_of("\n",found+1);	// Go to the next position were a "\n" is found.
	}

	frameUnwrapError += msg;	// Adds current "msg" into "frameUnwrapError".
	frameUnwrapError += "; ";	// Places a semicolon at the end of current "msg".
}

// Accumulates the ammount of master channel lost frames.
void TmChannelWarning::addMCLostFramesCount(uint16_t count)
{
	lostMCFrames += count;
}

// Accumulates the ammount of virtual channel lost frames.
void TmChannelWarning::addVCLostFramesCount(uint64_t count)
{
	lostVCFrames += count;
}

// Sets the packetResync flag to TRUE.
void TmChannelWarning::setPacketResynced()
{
	packetResync = true;
}

// Sets the noPacketSinkSpecified flag to TRUE.
void TmChannelWarning::setNoPacketSinkSpecified()
{
	noPacketSinkSpecified = true;
}

// Sets the noOcfSinkSpecified flag to TRUE.
void TmChannelWarning::setNoOcfSinkSpecified()
{
	noOcfSinkSpecified = true;
}

// Sets the unconfiguredVC flag to TRUE.
void TmChannelWarning::setUnconfiguredVC()
{
	unconfiguredVC = true;
}

// Sets the unconfiguredMC flag to TRUE.
void TmChannelWarning::setUnconfiguredMC()
{
	unconfiguredMC = true;
}

// Sets the recPacketBufferOverflow flag to TRUE.
void TmChannelWarning::setRecPacketBufferOverflow()
{
	recPacketBufferOverflow = true;
}

// Sets the recOcfBufferOverflow flag to TRUE.
void TmChannelWarning::setRecOcfBufferOverflow()
{
	recOcfBufferOverflow = true;
}

// Sets the wrongOcfFlag flag to TRUE.
void TmChannelWarning::setWrongOcfFlag()
{
	wrongOcfFlag = true;
}

// Sets the wrongScid flag to TRUE.
void TmChannelWarning::setWrongScid()
{
	wrongScid = true;
}

// Sets the wrongVcid flag to TRUE.
void TmChannelWarning::setWrongVcid()
{
	wrongVcid = true;
}

// Sets the wrongSecondHeaderFlag flag to TRUE.
void TmChannelWarning::setWrongSecondHeaderFlag()
{
	wrongSecondHeaderFlag = true;
}

// Sets the wrongSynchronisationFlag flag to TRUE.
void TmChannelWarning::setWrongSynchronisationFlag()
{
	wrongSynchronisationFlag = true;
}

// Removes newlines, replaces them with blank spaces, appends current msg into freeMessage and adds a semicolon at its end.
void TmChannelWarning::appendFreeMessage(string msg)
{
	// delete all newlines
	size_t found=msg.find_first_of("\n");	// Searches for the position of the first "\n" found in msg.
	while (found!=string::npos) {			// While the current position has not become the last position.
		msg[found]=' ';						// Replace the character in current position with a blank space.
		found=msg.find_first_of("\n",found+1);	// Go to the next position were a "\n" is found.
	}

	freeMessage += msg;		// Adds current "msg" into "freeMessage".
	freeMessage += "; ";	// Places a semicolon at the end of current "msg".
}

// Retrieves any and all errors and warnings stored and returns them as a single message - All non-empty error variables are then cleared.
string TmChannelWarning::popWarning()
{
	ostringstream msg;
	if (!frameUnwrapError.empty()) {
		msg << "Error while unwrapping the frame: " << frameUnwrapError;
		frameUnwrapError.clear();
	} else if (lostMCFrames > 0) {
		msg << "Lost " << dec << lostMCFrames << " master channel frames.";
		lostMCFrames = 0;
	} else if (lostVCFrames > 0) {
		msg << "Lost " << dec << lostVCFrames << " virtual channel frames.";
		lostVCFrames = 0;
	} else if (packetResync) {
		msg << "Packet resync.";
		packetResync = false;
	} else if (noPacketSinkSpecified) {
		msg << "No packet sink specified.";
		noPacketSinkSpecified = false;
	} else if (noOcfSinkSpecified) {
		msg << "No OCF sink specified.";
		noOcfSinkSpecified = false;
	} else if (unconfiguredVC) {
		msg << "Frame for unconfigured virtual channel received.";
		unconfiguredVC = false;
	} else if (unconfiguredVC) {
		msg << "Frame for unconfigured master channel received.";
		unconfiguredMC = false;
	} else if (recPacketBufferOverflow) {
		msg << "Buffer overflow in recieved packet buffer.";
		recPacketBufferOverflow = false;
	} else if (recOcfBufferOverflow) {
		msg << "Buffer overflow in received OCF buffer.";
		recOcfBufferOverflow = false;
	} else if (wrongOcfFlag) {
		msg << "Frame with wrong OCF flag received.";
		wrongOcfFlag = false;
	} else if (wrongScid) {
		msg << "Frame with wrong spacecraft ID received.";
		wrongScid = false;
	} else if (wrongVcid) {
		msg << "Frame with wrong virtual channel ID received.";
		wrongVcid = false;
	} else if (wrongSecondHeaderFlag) {
		msg << "Frame with wrong second header flag received.";
		wrongSecondHeaderFlag = false;
	} else if (wrongSynchronisationFlag) {
		msg << "Frame with wrong synchronisation flag received.";
		wrongSynchronisationFlag = false;
	} else if (!freeMessage.empty()) {
		msg << freeMessage;
		freeMessage.clear();
	}
	return msg.str();
}

// Checks if at least one of the possible warning causes has thrown a warning.
bool TmChannelWarning::warningAvailable()
{
	return ( !frameUnwrapError.empty()
		|| (lostMCFrames > 0)
		|| (lostVCFrames > 0)
		|| packetResync
		|| noPacketSinkSpecified
		|| noOcfSinkSpecified
		|| unconfiguredVC
		|| unconfiguredMC
		|| recPacketBufferOverflow
		|| recOcfBufferOverflow
		|| wrongOcfFlag
		|| wrongScid
		|| wrongVcid
		|| wrongSecondHeaderFlag
		|| wrongSynchronisationFlag
		|| !freeMessage.empty());
}
