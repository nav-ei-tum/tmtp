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
#include "OcfServer.h"
#include "TmMasterChannel.h"

// Constructor of the OcfServer class.
OcfServer::OcfServer()
{
	tmMc = NULL;		// The pointer to the master channel is initialized ot NULL.
	//tcMc = NULL;
}

// Specifies the master channel to which this OCF Server will be linked.
void OcfServer::connectTmMc (TmMasterChannel *mc)
{
	tmMc = mc;
}

// Sets the master channel pointer to NULL.
void OcfServer::disconnectTmMc ()
{
	tmMc = NULL;
}

/*void OcfServer::connectTcMc (TmMasterChannel *mc)
{
	tcMc = mc;
}*/

/*void OcfServer::disconnectTcMc ()
{
	tcMc = NULL;
}*/
