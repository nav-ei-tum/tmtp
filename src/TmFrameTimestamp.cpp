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
#include "TmFrameTimestamp.h"
#include <iostream>
#include <sstream>
#include <stdint.h>

using namespace std;

// Constructor of the Timestamp class.
TmFrameTimestamp::TmFrameTimestamp()
{
seconds = 0;
fractions = 0.0;
}

// Sets the seconds part of the timestamp. 
void TmFrameTimestamp::setSeconds(uint64_t secs)
{
	seconds = secs;
}

// Sets the fractions of seconds of the timestamp. 
void TmFrameTimestamp::setFractions(double fracs)
{
	if ((fracs >= 1.0)||( fracs < 0.0)) {	// Verifies if the fractions are smaller than 1 and greater than or equal to zero.
		ostringstream error;
		error << "The timestamp fractions fell out of bounds! They must be smaller than 1 and greater than or equal to zero." << endl;
		throw TmFrameTimestampError(error.str());
		}
	else {
		fractions = fracs;
		}
}

// Retrieves the seconds part of the timestamp.
uint64_t TmFrameTimestamp::getSeconds()
{
	return seconds;
}

// Retrieves the fractions of seconds of the timestamp.
double TmFrameTimestamp::getFractions()
{
	return fractions;
}

// Shows if a given timestamp has actual data measuring the amount of seconds after Epoch.
bool TmFrameTimestamp::isValid()
{
	/* Variable Seconds is initialized with a zero. 
	 * A timestamp counts the seconds ellapsed since Epoch. 
	 * Therefore, Seconds shall not be a zero if a timestamp was provided. 
	 */
	if (seconds==0) {
		return false;
		}
	else {
		return true;
		}
}

