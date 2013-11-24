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
#include "PacketServer.h"
#include "TmVirtualChannel.h"
#include "NetProtConf.h"

// Constructor of the PacketServer class.
PacketServer::PacketServer(NetProtConf *conf)
{
	netProtConf = conf;		// Network protocol configuration being used.
	tmVc = NULL;			// Initializes the Virtual Channel these packet belong to NULL.
	//tcVc = NULL;
}

// Establishes the Virtual Channel these packet will belong.
void PacketServer::connectTmVc (TmVirtualChannel *vc)
{
	tmVc = vc;
}

// Sets the Virtual Channel to NULL.
void PacketServer::disconnectTmVc ()
{
	tmVc = NULL;
}

/*void PacketServer::connectTcVc (TcVirtualChannel *vc)
{
	tcVc = vc;
}*/

/*void PacketServer::disconnectTcVc ()
{
	tcVc = NULL;
}*/
