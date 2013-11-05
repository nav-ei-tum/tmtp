#include "myErrors.h"
#include "TmFrameBitrate.h"
#include <iostream>
#include <sstream>
#include <stdint.h>

using namespace std;

// Constructor of the Bitrate class.
TmFrameBitrate::TmFrameBitrate()
{
	bps = bpsInitPattern;
}

// Sets the bitrate in bits per second. 
void TmFrameBitrate::setBitrate(double rate)
{
	if ((rate < 0)) {	// Verifies if the received bitrate is a positive value.
		ostringstream error;
		error << "Received bitrate is negative!" << endl;
		throw TmFrameBitrateError(error.str());
		}
	else {
		bps = rate;
		}
}

// Retrieves the bitrate.
double TmFrameBitrate::getBitrate()
{
	return bps;
}

// Shows if a given bitrate has actual data.
bool TmFrameBitrate::isValid()
{
	if (bps == bpsInitPattern) {
		return true;
		}
	else {
		return false;
		}
}

