#include "TmOcf.h"
#include "myErrors.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <stdint.h>

using namespace std;

// Constructor for the TmOcf class.
TmOcf::TmOcf()
{
	reportType = Type1Clcw;
	content = 0;
}

// Sets the Report Type of the OCF.
void TmOcf::setReportType(ReportType type)
{
	reportType = type;
}

// Gets the current Report Type of the OCF.
TmOcf::ReportType TmOcf::getReportType()
{
	return reportType;
}

// Receives data and stores it as a report type to the OCF.
void TmOcf::setContent(uint64_t data)
{ 
	// First we check the data fits the size of its report type
	if (reportType == Type1Clcw) {
		if (data >= 0x7FFFFFFF) {					// The contents of the Type-1-Report must occupy 31 bits or less
			ostringstream error;
			error << "Content too large." << endl;
			throw TmOcfError(error.str());
		}
	} else {
		if (data >= 0x3FFFFFFF) {					// The contents of the Type-2-Report must occupy 30 bits or less
			ostringstream error;
			error << "Content too large." << endl;
			throw TmOcfError(error.str());
		}
	}
	content = data; // Data of appropriate size is assigned to the OCF content
}

// Retrieves the contents of the last 31 or 30 bits of the OCF.
uint64_t TmOcf::getContent()
{
	return content;
}

// Copies the content of "content" into a 4-Byte vector called "raw" and modifies its 1st and 2nd bits according to the Report Type.
// The "raw" vector (32 bits) is the OCF contents (31 or 30 bits) and the Report Type flags (1 or 2 bits) together.
vector<uint8_t> TmOcf::wrap()
{
	vector<uint8_t> raw (ocfLength, 0); // Creates a vector of four Bytes, fills it with zeroes and calls it "raw".

	uint8_t firstByte = 0;
	switch (reportType) {
		case Type1Clcw:								// If Type-1-Report, 
			firstByte = 0x00;						
			firstByte |= (content >> 24) & 0x7F;	// takes the 1st Byte of the contents, sets its 1st bit to '0' and stores all in firstByte.
			break;
		case Type2ProjectSpecific:					// If Type-2-Report - Project Specific, 
			firstByte = 0x80;						// assigns the 1st bit = '1', 2nd bit = '0',
			firstByte |= (content >> 24) & 0x3F;	// takes the 1st Byte of the contents, sets its 1st bit = '0', 2nd bit = '0' and stores all in firstByte.
			break;
		case Type2FutureReserved:					// If Type-2-Report - Reserved for Future Applications, 
			firstByte = 0xC0;						// assigns the 1st and 2nd bits a '1',
			firstByte |= (content >> 24) & 0x3F;	// takes the 1st Byte of the contents, sets its 1st and 2nd bits to '1' and stores all in firstByte.
			break;
		default:
			break;
	}
	
	// Now we copy every Byte in the "content" vector to the "raw" vector. The first Byte gets the contents of firstByte.
	raw[0] = firstByte;
	raw[1] = (content >> 16) & 0xFF;
	raw[2] = (content >> 8) & 0xFF;
	raw[3] = content & 0xFF;

	return raw;
}

// Takes a raw vector (Report Type flags + contents), reads and sets the flags to '0', sets "reportType" accordingly and stores all back into "content".
void TmOcf::unwrap(vector<uint8_t> raw)
{
	if (raw.size() != ocfLength) {	// Checks if the received raw vector is 4 Bytes long.
		ostringstream error;
		error << "Wrong OCF length. ";
		error << dec << raw.size() << " bytes received, instead of " << ocfLength << "." << endl;
		throw TmOcfError(error.str());
	}
	if ((raw[0] & 0x80) == 0) {				// If the 1st bit of "raw" is '0':
		reportType = Type1Clcw;				// sets the Report Type to Type-1-Report,
		content = ((raw[0] & 0x7F) << 24);	// "removes" the 1st bit of the 1st Byte of "raw" and 
											// stores the whole byte in the 1st position of "content".

	} else if ((raw[0] & 0x40) == 0) {		// If the first two bits of "raw" are "10":
		reportType = Type2ProjectSpecific;	// sets the Report Type to Type-2-Report - Project Specific,
		content = ((raw[0] & 0x3F) << 24);	// "removes" the first two bits of the 1st Byte of "raw" and 
											// stores the whole Byte in the 1st position of "content".

	} else {								// If the first two bits of "raw" are "11":
		reportType = Type2FutureReserved;	// sets the Report Type to Type-2-Report - Reserved for Future Applications,
		content = ((raw[0] & 0x3F) << 24);	// "removes" the first two bits of the 1st Byte of "raw" and 
											// stores the whole Byte in the 1st position of "content".
	}
	
	// Now we transfer the contents of the remaining Bytes of "raw" into the corresponding positions of "content".
	content |= (raw[1] << 16);
	content |= (raw[2] << 8);
	content |=  raw[3];
}

// Displays a message (for debugging) with the Report Type and OCF contents in hexadecimal.
void TmOcf::debugOutput()
{
	cout << "OCF of ";
	switch (this->getReportType()) {		// Reads the Report Type and
		case TmOcf::Type1Clcw:				// displays the adequate Type...
			cout << "Type1Clcw";
			break;
		case TmOcf::Type2ProjectSpecific:
			cout << "Type2ProjectSpecific";
			break;
		case TmOcf::Type2FutureReserved:
			cout << "Type2FutureReserved";
			break;
		default:
			break;
	}
	cout << ": " << hex << this->getContent() << endl; // ... Followed by the actual OCF contents in hexadecimal.
}
