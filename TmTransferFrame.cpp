#include "TmTransferFrame.h"
#include "TmOcf.h"
#include "myErrors.h"
#include "TmFrameTimestamp.h"
#include "TmFrameBitrate.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdint.h>

using namespace std;

// Constructor for the TmTransferFrame class.
TmTransferFrame::TmTransferFrame(uint16_t length)
{
	if ((length >= 7) && (length <= 2048)) {	// Verifies if the total frame length (in Bytes) given falls within the required range.
		frameLength = length;
	} else {
		ostringstream error;
		error << "Frame length out of range (7-2048)." << endl;
		throw TmTransferFrameError(error.str());
	}
	
	// After a new frame is generated, the following values become the defaults:
    spacecraftId = 0;
    virtualChannelId = 0;
    ocfPresent = false;				// This is an optional field. Part of the Telemetry Frame Trailer.
    masterChannelFrameCount = 0;	// Resets the Master Channel Frame Counter.
    virtualChannelFrameCount = 0;	// Resets the Virtual Channel Frame Counter.
    secondHeaderPresent = false;	// Will NOT use the TM Secondary Header,
	extendedVcFrameCount = false;	//	and therefore no extended VC Frame Counter will be used.
    dataFieldSynchronised = false;	// The packet format in the TM Data Field will NOT be Byte-syncrhonized, forward-ordered.
    firstHeaderPointer = 0;			// Initializes the First Header Pointer to 0000 0000 0000 0000.
	fecfPresent = false;			// No Frame Error Control Field present.
}

// Retrieves the Transfer Frame Version Number.
uint16_t TmTransferFrame::getTransferFrameVersion()
{
	return transferFrameVersion;
}

// Sets the Spacecraft Identifier.
void TmTransferFrame::setSpacecraftId(uint16_t id)
{
	// The length is verified. The Spacecraft ID is 10 bits long, so it must fall within the range of 0 to 1023.
	if (id < 1024) {
		spacecraftId = id;
	} else {
		ostringstream error;
		error << "Spacecraft ID out of range (0-1023)." << endl;
		throw TmTransferFrameError(error.str());
	}
}

//Retrieves the Spacecraft Identifier.
uint16_t TmTransferFrame::getSpacecraftId()
{
	return spacecraftId;
}

// Sets the Virtual Channel Identifier.
void TmTransferFrame::setVirtualChannelId(uint16_t id)
{
	// The length is verified. The Virtual Channel ID is 3 bits long, so it must fall within the range of 0 to 7.
	if (id < 8) {
		virtualChannelId = id;
	} else {
		ostringstream error;
		error << "Virtual Channel ID out of range (0-7)." << endl;
		throw TmTransferFrameError(error.str());
	}
}

// Retrieves the Virtual Channel Identifier.
uint16_t TmTransferFrame::getVirtualChannelId()
{
	return virtualChannelId;
}

// Sets the Operational Control Field Flag to TRUE.
void TmTransferFrame::activateOcf()
{
	ocfPresent = true;
}

// Retrieves the value of the Operational Control Field Flag.
bool TmTransferFrame::getOcfStatus()
{
	return ocfPresent;
}

// Inserts a 4-Byte long Operational Control Field object.
void TmTransferFrame::setOcf(TmOcf ocfObject)
{
	ocf = ocfObject;
}

// Retrieves the Operational Control Field currently in the TF Trailer.
TmOcf TmTransferFrame::getOcf()
{
	return ocf;
}

// Sets the Frame Error Control Field flag to TRUE.
void TmTransferFrame::activateFecf()
{
	fecfPresent = true;
}

// Retrieves the value of the Frame Error Control Field flag.
bool TmTransferFrame::getFecfStatus()
{
	return fecfPresent;
}

// Sets the Master Channel Frame Counter.
void TmTransferFrame::setMasterChannelFrameCount(uint16_t count)
{
	// The length is verified. The Master Channel Frame Counter is 8 bits long, so it must fall within the range of 0 to 255.
	if (count < 256) {
		masterChannelFrameCount = count;
	} else {
		ostringstream error;
		error << "Master Channel frame count out of range (0-255)." << endl;
		throw TmTransferFrameError(error.str());
	}
}

// Retrieves the Master Channel Frame Counter.
uint16_t TmTransferFrame::getMasterChannelFrameCount()
{
	return masterChannelFrameCount;
}

// Sets the Virtual Channel Frame Counter.
void TmTransferFrame::setVirtualChannelFrameCount(uint64_t count)
{
	// The length is verified. The Virtual Channel Frame Counter is 8 bits long, so it must fall within the range of 0 to 255.
	if (extendedVcFrameCount) {
		virtualChannelFrameCount = count;
	} else {
		if (count < 256) {
			virtualChannelFrameCount = count;
		} else {
			ostringstream error;
			error << "Virtual Channel frame count out of range (0-255)." << endl;
			throw TmTransferFrameError(error.str());
		}
	}
}

// Retrieves the Virtual Channel Frame Counter.
uint64_t TmTransferFrame::getVirtualChannelFrameCount()
{
	return virtualChannelFrameCount;
}

// Sets the Secondary Header flag to TRUE.
void TmTransferFrame::activateSecondHeader()
{
	secondHeaderPresent = true;
}

// Retrieves the value of the Secondary Header flag.
bool TmTransferFrame::getSecondHeaderStatus()
{
	return secondHeaderPresent;
}

// Activates the extended VC Frame Counter, fixes the SH Data Field (if empty) to three Bytes and fills it with zeroes.
void TmTransferFrame::activateExtendedVcFrameCount()
{
	extendedVcFrameCount = true;			// Activates the extended VC Frame Counter.
	if (secondHeaderDataField.empty()) {
		this->activateSecondHeader();		// If the Secondary Header Data Field is empty, it activates it.
		secondHeaderDataField.assign(3,0);	// Assigns three Bytes with a value of "0" to the vector, replacing its current contents.
	} else {
		if (!secondHeaderPresent) {	// If the Secondary Header Data Field has some data but the Secondary Header flag is FALSE,
			ostringstream error;	// then we have some weird error.
			error << "No second header present but extended VC frame count configured." << endl;
			throw TmTransferFrameError(error.str());
		}
		if (secondHeaderDataField.size() != 3) {	// If the Secondary Header Data Field has some data, it checks if it is 3 Bytes long.
													// A length of 3 bytes could mean that the contents are an extended counter.
			ostringstream error;					// In this the case, it DOES NOT FORCE the extended VC Frame Counter activation.
													// Instead, it preserves its contents and copies them into the last three Bytes of the VC Frame Counter. 
													// Otherwise, an error is thrown.
			error << "Wrong second header length for extended VC frame count." << endl;
			throw TmTransferFrameError(error.str());
		}
		// So, if the Secondary Header flag is TRUE and the SH Data Field length is 3 Bytes, then we most likely have some Extended Counter data in there already.
		virtualChannelFrameCount |= (secondHeaderDataField[0] << 24);	// The 1st Byte of the SH Data Field is OR'd to the most significant Byte of the VC Frame Counter,
		virtualChannelFrameCount |= (secondHeaderDataField[1] << 16);	// the 2nd Byte of the SH Data Field is OR'd to the 2nd msB of the VC Frame Counter,
		virtualChannelFrameCount |= (secondHeaderDataField[2] << 8);	// the 3rd Byte of the SH Data Field is OR'd to the 3rd msB of the VC Frame Counter...
																		// and the last Byte of the VC Frame Counter (the original counter) is left untouched.
	}
}

// Retrieves the value of the Extended VC Frame Conter flag.
bool TmTransferFrame::getExtendedVcFrameCountStatus()
{
	return extendedVcFrameCount;
}

// Retrieves the value of the Secondary Header Version.
uint16_t TmTransferFrame::getSecondHeaderVersion()
{
	return secondHeaderVersion;
}

// Retrieves the length of the Secondary Header Data Field.
uint16_t TmTransferFrame::getSecondHeaderLength()
{
	return secondHeaderDataField.size() + 1; // The +1 takes into account the lengths of Version Number and the Length Fields.
}

// Populates the Secondary Header Data Field.
void TmTransferFrame::setSecondHeaderDataField(vector<uint8_t> data)
{
	if (extendedVcFrameCount) {	// If the Extended VF Frame Counter is active, this Data Field shall NOT be used for any other purpose.
		ostringstream error;
		error << "Assignment of second header data field is invalid if extended VC frame count is used." << endl;
		throw TmTransferFrameError(error.str());
	}
	if ((uint16_t) data.size() > (this->getMaxSecondHeaderLength() - 1)) {	// If somehow the data to be stored needs more than the max, an error is thrown.
		ostringstream error;
		error << "Second header data field is too long." << endl;
		throw TmTransferFrameError(error.str());
	}
	secondHeaderDataField = data;	// If none of the above is the case, then we store "data" into the SH Data Field.
}

// Retrieves the contents of the Secondary Header Data Field.
vector<uint8_t> TmTransferFrame::getSecondHeaderDataField()
{
	return secondHeaderDataField;
}

// Sets the Synchronization Flag to TRUE.
void TmTransferFrame::activateDataFieldSynchronisation()
{
	dataFieldSynchronised = true;
}

// Retrieves the value of the Synchronization Flag.
bool TmTransferFrame::getDataFieldSynchronisationStatus()
{
	return dataFieldSynchronised;
}

// Sets the location in the TM Data Field where the 1st Byte of a new packet starts.
void TmTransferFrame::setFirstHeaderPointer(uint16_t location)
{
	uint16_t maxLocation = this->getDataFieldLength() - 1;	// We set the higher boundary by calculating the Data Field Length and subtracting 1. 

	if ((location <= maxLocation) || (location==fhpNoFirstHeader)	// If the location specified does not go beyond the higher limit,  
			|| (location==fhpOnlyIdleData)) {						// or if it matches the predefined patterns for no-first-header or idle data...
		firstHeaderPointer = location;								// It becomes the new First Header Pointer!
	} else {
		ostringstream error;										// Otherwise we are throwing an out-of-bounds error message.
		error << "First header pointer out of range (0-" << maxLocation << ",";
		error << fhpNoFirstHeader << "," << fhpOnlyIdleData << ")." << endl;
		throw TmTransferFrameError(error.str());
	}
}

// Retrieves the value of the First Header Pointer.
uint16_t TmTransferFrame::getFirstHeaderPointer()
{
	return firstHeaderPointer;
}

// Retrieves the length of the whole TM Transfer Frame.
uint16_t TmTransferFrame::getLength()
{
	return frameLength;
}

// Calculates the Data Field length.
uint16_t TmTransferFrame::getDataFieldLength()
{
	uint64_t length = frameLength;						// As a precaution, the total frame length is copied into a local (signed) variable.
	length -= primaryHeaderLength;					// First, we subtract the Primary Header length (constant 6 Bytes).
	if (secondHeaderPresent) {
		length -= this->getSecondHeaderLength();	// Second, if a Secondary Header is present, it will be subtracted.
	}
	if (ocfPresent) {								// Third, if the Trailed fields are present, we start removing the Operational Control Field
		length -= TmOcf::ocfLength;
	}
	if (fecfPresent) {								// and finish by removing the Frame Error Control Field.
		length -= fecfLength;
	}

	uint16_t ret;
	if (length > 0) {						// If after all the subtractions we end up with a positive integer,
		ret = (uint16_t) length;			// the length is casted as a two-Byte long number.
	} else {
		ret = 1;							// Otherwise we just say the Data Field length is 1.
	}
	return ret;			// And the result is returned as the Data Field Length.
}

// Calculates the Secondary Header length.
uint16_t TmTransferFrame::getMaxSecondHeaderLength()
{
	uint64_t length = frameLength;		// As a precaution, the total frame length is copied into a local (signed) variable.
	length -= primaryHeaderLength;	// First, we subtract the Primary Header length.
	if (ocfPresent) {
		length -= TmOcf::ocfLength;	// Second, if the Trailed fields are present, we start removing the Operational Control Field
	}
	if (fecfPresent) {				// and then by removing the Frame Error Control Field.
		length -= fecfLength;
	}
	length -= 1; 					// We finish by subtracting 1, to account for the TM Data Field minimum size.

	uint16_t ret;
	if (length > 0) {					// If after all the subtractions we end up with a positive integer,
		ret = (uint16_t) length;		// the length is casted as a two-Byte long number.
	} else {
		ret = 0;						// Otherwise we say there is no secondary header (length is zero).
	}
	if (ret > 64) { 					// The maximum length allowed by standard is 64, so we truncate higher lengths.
		ret = 64;
	}
	return ret;
}

// Populates the TM Data Field.
void TmTransferFrame::setDataField(vector<uint8_t> data)
{
	if (data.size() == this->getDataFieldLength()) {
		dataField = data;			// Only if the provided data is the exact same length as the calculated field length, then we store it there.
									// This (protected) variable will be available for all of the other function members in this class.
	} else {
		ostringstream error;
		error << "Data field has wrong size. It is " << data.size() << " but should be ";
		error << this->getDataFieldLength() << " bytes long." << endl;
		throw TmTransferFrameError(error.str());
	}
}

// Retrieves the data stored in the TM Data Field as obtained from the unwrap() function.
vector<uint8_t> TmTransferFrame::getDataField()
{
	vector<uint8_t> retVec;	// We create a retrieval vector of length X and undefined contents.
	
	if (this->getDataFieldLength() > 0) {		// The Data Field length is calculated and if it is not zero,
		retVec = dataField;						// the contents of dataField are stored in the retrieval vector.
		long dif = this->getDataFieldLength() - retVec.size();		// We calculate the difference btw. length X and the Data Field length.
		if (dif > 0) {
			retVec.insert(retVec.end(),dif,0);				// If the Data Field length is greater, the diference in retVec is filled with zeroes.
		} else if (dif < 0) {
			retVec.erase(retVec.end()+dif,retVec.end());	// If the length of retVec is greater, the diference in retVec is truncated.
		}							// This difference of length could be generated at the declaration of the variable.
	return retVec;	// We return the retrieval vector.
	}
	else {
		ostringstream error;
		error << "Invalid Data Field size (less than 1 Byte).";
		throw TmTransferFrameError(error.str());
	}
}

// Builds the different frame fields and encapsulates a packet in it.
vector<uint8_t> TmTransferFrame::wrap()
{
	vector<uint8_t> raw; // A vector is created into which the whole frame will be assembled, piece by piece.

	// The wrap begins by checking if the Data Field length is correct.
	// In order to correctly calculate the Data Field length, 
	// the optional fields (if needed) must be activated first. Follow this sequence:

	// 1.- The Secondary Header: 
	//	- TmTransferFrame::activateSecondHeader()
	
	// 2.- Extended Virtual Circuit Frame Counter:
	//	- TmTransferFrame::activateExtendedVcFrameCount()
	
	// 3.- The Operational Control Field:
	//	- TmOcf::setReportType(ReportType type)
	//	- TmOcf::setContent(uint64_t data)
	//	- TmOcf::wrap()
	//	- TmTransferFrame::activateOcf()
	
	// 4.- The Frame Error Control Field:
	//	- TmTransferFrame::activateFecf()
	
	if (this->getDataFieldLength() == 0) {	// The Data Field length is calculated and checked.
		ostringstream error;
		error << "Frame too short to carry all configured information." << endl;
		throw TmTransferFrameError(error.str());
	}

	// The first two Bytes of the Primary Header are set up.
	uint16_t headerFirstPart = 0;
	headerFirstPart |= (transferFrameVersion & 0x0003) << 14; // version is encoded in the first 2 bits
	headerFirstPart |= (spacecraftId & 0x03FF) << 4;          // scid is encoded in the next 10 bits
	headerFirstPart |= (virtualChannelId & 0x0007) << 1;      // vcid is encoded in the next 3 bits
	if (ocfPresent) {
		headerFirstPart |= 0x0001;                            // ocfFlag is encoded in the last bit
	}

	// We skip the Master and Virtual Channel Frame Counters for now and move right onto...
	
	// ... The Data Field Status (2 Bytes) is set up.
	uint16_t dataFieldStatus = 0;
	if (secondHeaderPresent) {
		dataFieldStatus |= 0x0001 << 15;                // secondary header flag is encoded in the first bit
	}
	if (dataFieldSynchronised) {                        // synchronisation flag = 0 (wait, what??); packet order flag = 0;
	                                                    //   segment length identifier = 0b11;
	                                                    //   first header pointer = fhpNoFirstHeader = all 1's;
		dataFieldStatus |= 0x0003 << 11;                // segment length identifier is encoded in bits 4-5
		dataFieldStatus |= firstHeaderPointer & 0x07FF; // first header pointer is encoded in the last 11 bits
		
		// CAUTION! The usage of dataFieldSynchronised is misleading:
		//			Here it is implied that when dataFieldSynchronised = TRUE, it means the Synchronization Flag is FALSE!
		//			Refer to ECSS-E-ST-50-03C, clause 5.2.7.3 for more details.
		
	} else {                                            // synchronisation flag = 1; rest undefined (filled with 0's here)
		dataFieldStatus |= 0x0001 << 14;                // synchronization flag is encoded in the second bit
	}

	// Secondary Header (optional) is set up.
	uint16_t secondHeaderId = 0;
	if (secondHeaderPresent) {
		secondHeaderId |= (secondHeaderVersion & 0x0003) << 6;        // version is encoded in the first 2 bits
		
		// extended virtual channel frame count
		if (extendedVcFrameCount) {
			secondHeaderDataField.assign(3,0);									// Assigns three Bytes of all zeroes.
			secondHeaderDataField[0] = (virtualChannelFrameCount >> 24) & 0xff;	// The msB of the counter is stored in the 1st Byte of the SH Data Field 
			secondHeaderDataField[1] = (virtualChannelFrameCount >> 16) & 0xff;	// The 2nd msB into the 2nd Byte of the SH Data Field 
			secondHeaderDataField[2] = (virtualChannelFrameCount >> 8) & 0xff;	// And finally the 3rd msB is stored in the 3rd Byte of the SH Data Field 
		}
		secondHeaderId |= (this->getSecondHeaderLength()-1) & 0x003F; // length-1 is encoded in the last 6 bits
	}

	// Now we put everything together:
	raw.push_back(headerFirstPart >> 8 );					// The 1st Byte of the Primary Header is inserted.
	raw.push_back(headerFirstPart & 0x00FF);				// The 2nd Byte of the Primary Header is inserted.
	
	raw.push_back(masterChannelFrameCount & 0x00FF);		// The Master Channel Frame Counter is inserted.
	raw.push_back(virtualChannelFrameCount & 0x000000FF);	// The Virtual Channel Frame Counter is inserted.
	
	raw.push_back(dataFieldStatus >> 8 );					// The 1st Byte of the Data Field Status is inserted.
	raw.push_back(dataFieldStatus & 0x00FF);				// The 2nd Byte of the Data Field Status is inserted.
	
	if (secondHeaderPresent) {								// If the Secondary Header is going to be used:
		raw.push_back(secondHeaderId & 0x00FF);				// The Secondary Header Id is inserted.
		raw.insert(raw.end(), secondHeaderDataField.begin(), secondHeaderDataField.end());	// The SH Data Field is inserted.
	}
	
	vector<uint8_t> tmpDataField = this->getDataField();
	raw.insert(raw.end(), tmpDataField.begin(), tmpDataField.end());	// The packet is inserted in the TM Data Field.

	if (ocfPresent) {											// Of the Operational Control Field is going to be used:
		vector<uint8_t> rawOcf = ocf.wrap();				// The Operational Control Field is wrapped.
		raw.insert(raw.end(), rawOcf.begin(), rawOcf.end());	// The raw Operational Control Field is inserted.
	}
	
	if (fecfPresent) {						// If the Frame Error Control Field is going to be used:
		uint16_t FECF = crc(raw);		// A CRC is computed from what we have stored in the raw vector.
		raw.push_back(FECF >> 8);			// The 1st Byte of the CRC is inserted.
		raw.push_back(FECF & 0x00FF);		// The 2nd Byte of the CRC is inserted and we're done!
	}

	return raw;	// A new TMTP Frame is born!
}

// Takes a TMTP Frame, reads the Fields and Flags and stores their values in local variables accordingly.
void TmTransferFrame::unwrap(vector<uint8_t> raw)
{
	if (raw.size() != frameLength) {	// All frames must be fixed length, so the received frame should match the established length.
		ostringstream error;
		error << "Wrong frame length. ";
		error << dec << raw.size() << " bytes instead of " << frameLength << "." << endl;
		throw TmTransferFrameError(error.str());
	}

	if (fecfPresent) {					// If there is a Frame Error Control Field present, a CRC is computed against the whole frame.
		if (crc(raw) != 0) {
			cout << "Checksum error, received packet: ";
			for(unsigned int i=0;i<raw.size();i++) {
				cout << hex << (unsigned int)raw[i];
				cout << " ";
			}
			cout << endl;
			throw TmTransferFrameError("Checksum error");
		}
	}

	uint16_t headerFirstPart = (raw[0] << 8) | raw[1];	// The 1st 2 Bytes are extracted (Master- and Virtual Channel IDs and OCF Flag).
	uint16_t recTransferFrameVersion = ((headerFirstPart >> 14) & 0x0003);	// The TF Version Number is extracetd from the Master Channel ID.
	if (recTransferFrameVersion != transferFrameVersion) {							// The received version is compared to the established one.
		ostringstream error;
		error << "Unsupported frame version " << dec << recTransferFrameVersion << "." << endl;
		throw TmTransferFrameError(error.str());
	}

	spacecraftId = (headerFirstPart >> 4) & 0x03FF;			// The Spacecraft Id is extracted.
	virtualChannelId = (headerFirstPart >> 1) & 0x0007;		// The Virtual Channel ID is extracted.
	ocfPresent = headerFirstPart & 0x0001;					// The Operational Control Field Flag is extracted.

	masterChannelFrameCount = raw[2];	// The Master Channel Frame Counter is extracted.
	virtualChannelFrameCount = raw[3];	// The Virtual Channel Frame Counter is extracted (Note: the extension is extracted later, if any).

	uint16_t dataFieldStatus = (raw[4] << 8) | raw[5];		// The Data Field Status is extracted (2 Bytes).

	secondHeaderPresent = (dataFieldStatus >> 15) & 0x0001;			// The TF Secondary Header Flag is extracted.
	dataFieldSynchronised = !((dataFieldStatus >> 14) & 0x0001);	// The (misleading) Synchronization Flag is extracted.
	if (dataFieldSynchronised) {						// If using Byte Synchronization format...
		firstHeaderPointer = dataFieldStatus & 0x07FF;	// ... The First Header Pointer is extracted.
	}

	if (secondHeaderPresent) {								// If a Secondary Header is present:
		uint16_t secondHeaderId = raw[primaryHeaderLength];	// The Secondary Header ID is extracted.
		uint16_t recSecondHeaderVersion = (secondHeaderId >> 6) & 0x0003;	// The version is extracted.
		if (recSecondHeaderVersion != secondHeaderVersion) {		// The received version is compared against the estalished version.
			ostringstream error;
			error << "Unsupported secondary header version " << dec << recSecondHeaderVersion << "." << endl;
			throw TmTransferFrameError(error.str());
		}
		uint16_t secondHeaderLength = (secondHeaderId & 0x003F) + 1;	// The Secondary Header Length is extracted and increased by 1.
		if (secondHeaderLength > this->getMaxSecondHeaderLength()) {		// The current SH Length is calculated and compared to the length received.
			ostringstream error;
			error << "Second Header too long.";
			throw TmTransferFrameError(error.str());
		}
		secondHeaderDataField.assign(raw.begin()+primaryHeaderLength+1,	// The Secondary Header Field is extracted.
			raw.begin()+primaryHeaderLength+secondHeaderLength);
		if (extendedVcFrameCount) {						// If using an extended VC Frame Counter...
			if (secondHeaderDataField.size() != 3) {	// ... Its length should be of three Bytes.
				ostringstream error;
				error << "Wrong second header length for extended VC frame count." << endl;
				throw TmTransferFrameError(error.str());
			}
			
			// The most significant three Bytes of the counter are added to the already extracted least significant Byte.
			virtualChannelFrameCount |= (secondHeaderDataField[0] << 24);
			virtualChannelFrameCount |= (secondHeaderDataField[1] << 16);
			virtualChannelFrameCount |= (secondHeaderDataField[2] << 8);
		}
	} else if (extendedVcFrameCount) {	// If no Secondary Header is present but the Extended VC Frame Counter flag was activated
		ostringstream error;			// then we have a funny error.
		error << "No second header present but extended VC frame count configured." << endl;
		throw TmTransferFrameError(error.str());
	}

	if (this->getDataFieldLength() == 0) {	// The Data Field length is calculated and should not be zero.
		ostringstream error;
		error << "Frame too short for configured features." << endl;
		throw TmTransferFrameError(error.str());
	}
	dataField.assign(raw.begin()+getDataFieldStart(), raw.begin()+getDataFieldEnd());	// The Data Field is extracted.

	if (ocfPresent) {
		vector<uint8_t> rawOcf (raw.begin()+getDataFieldEnd(),	// The Operational Control Field is extracted.
			raw.begin()+getDataFieldEnd()+TmOcf::ocfLength);
		try {
			ocf.unwrap(rawOcf);		// The unwrap function of the TmOcf class is used.
		} catch (TmOcfError& e) {
			ostringstream error;
			error << "Error in TmOcf: " << e.what() << endl;
			throw TmTransferFrameError(error.str());
		}
	}
}

// Dissects the frame into its components and displays them as messages for debugging.
void TmTransferFrame::debugOutput()
{
	cout << "TmTransferFrame";
	cout << "[" << dec << this->getLength() << "] ";
//	cout << "Ver: " << dec << this->getTransferFrameVersion() << ", ";
//	cout << "SCID: " << dec << setw(4) << this->getSpacecraftId() << ", ";
	cout << "VCID: " << dec << this->getVirtualChannelId() << ", ";
	cout << "OCF: " << boolalpha << setw(5) << this->getOcfStatus() << ", ";
	cout << "MCFC: " << dec << setw(3) << this->getMasterChannelFrameCount() << ", ";
	cout << "VCFC: " << dec << setw(3) << this->getVirtualChannelFrameCount() << ", ";
	cout << "2H: " << boolalpha << setw(5) << this->getSecondHeaderStatus() << ", ";
//	cout << "EVCFC: " << boolalpha << setw(5) << this->getExtendedVcFrameCountStatus() << ", ";
//	cout << "Sync: " << boolalpha << setw(5) << this->getDataFieldSynchronisationStatus() << ", ";
	cout << "FHP: " << dec << setw(4) << this->getFirstHeaderPointer() << ", ";
	cout << "FECF: " << boolalpha << setw(5) << this->getFecfStatus() << ", ";
	cout << "Content: \"";
	for (uint16_t i=0; i < this->getDataFieldLength(); i++) {
		cout << dataField[i];
	}
	cout << "\"[" << dec << this->getDataFieldLength() << "]";
	cout << endl;
}

// Calculates the position (in Bytes) where the TM Data Field starts.
uint16_t TmTransferFrame::getDataFieldStart()
{
	uint16_t start = primaryHeaderLength;
	if (secondHeaderPresent) {
		start += this->getSecondHeaderLength();
	}
	return start;
}

// Calculates the position (in Bytes) where the TM Data Field ends.
uint16_t TmTransferFrame::getDataFieldEnd()
{
	uint16_t end = frameLength;
	if (ocfPresent) {
		end -= TmOcf::ocfLength;
	}
	if (fecfPresent) {
		end -= fecfLength;
	}
	return end;
}

// Computes a Cyclic Redundancy Check on a message (the whole frame except the FECF). 
uint16_t TmTransferFrame::crc(vector<uint8_t> message)
{
	uint16_t sr = 0xFFFF;		// We initialize a two-Byte Shift Register full of 1's.
	for (uint16_t i=0; i < message.size(); i++) {
		uint16_t byte = message[i];				// We take the one Byte of the message.
		for (uint16_t bit=0; bit < 8; bit++) {	
			uint16_t newBit = byte >> (7 - bit);	// 
			uint16_t add = ((sr >> 15) ^ newBit) & 0x0001;
			add |= (add << 5) | (add << 12);
			sr = (sr << 1) ^ add;
		}
	}
	return sr;
}

// Inserts a timestamp object.
void TmTransferFrame::setTimestamp(TmFrameTimestamp timestampObject)
{
referenceTimestamp = timestampObject;
}

// Retrieves the stored TmFrameTimestamp object.
TmFrameTimestamp TmTransferFrame::getTimestamp()
{
return referenceTimestamp;
}

// brief Inserts a bitrate object.
void TmTransferFrame::setBitrate(TmFrameBitrate bitrateObject)
{
referenceBitrate = bitrateObject;
}

// Retrieves the stored TmFrameBitrate object.
TmFrameBitrate TmTransferFrame::getBitrate()
{
return referenceBitrate;
}
