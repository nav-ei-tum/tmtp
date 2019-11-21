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
#include "GroundOcfServer.h"
#include "TmMasterChannel.h"
#include "TmOcf.h"
#include "myErrors.h"
#include <iostream>
#include <sstream>

using namespace std;

// Constructor of the Ground OCF Server class.
GroundOcfServer::GroundOcfServer()
{
	debugOutput = false;
}

// Retrieves the 1st OCF message in the reception queue and optionally displays the corresponding debug messages.
void GroundOcfServer::signalNewOcf()
{
	if (tmMc) {								// If this OCF Server has been "connected" to a master channel,
		while (tmMc->ocfAvailable()) {		// and while there are any (received) OCF messages in the OCF input queue:
			try {
				TmOcf ocf = tmMc->receiveOcf();		// Retrieves the 1st OCF message in the input queue and
				if (debugOutput) {					// if the debug output flag is TRUE, displays the corresponding debug messages.
					cout << "Received " << flush;
					ocf.debugOutput();
				}
			} catch (TmMasterChannelError& e) {		// Any errors generated while instantiating the OCF object
				ostringstream error;				// are thrown as GroundOcfServer errors.
				error << "Error in TmMasterChannel: " << e.what() << endl;
				throw GroundOcfServerError(error.str());
			}
		}
	} else {								// If no master channel has been "connected" to this OCF Server,
		ostringstream error;				// throws a GroundOcfServer error.
		error << "No TmMasterChannel specified." << endl;
		throw GroundOcfServerError(error.str());
	}
}

// Sets the debugOutput flag to TRUE.
void GroundOcfServer::activateDebugOutput()
{
	debugOutput = true;
}

// Sets the debugOutput flag to FALSE.
void GroundOcfServer::deactivateDebugOutput()
{
	debugOutput = false;
}
