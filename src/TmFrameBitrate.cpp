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
#include "TmFrameBitrate.h"
#include <iostream>
#include <sstream>
#include <stdint.h>

using namespace std;

const double TmFrameBitrate::bpsInitPattern = 0x0F0F0F0F0F0F0F0F;

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

