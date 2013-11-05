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
