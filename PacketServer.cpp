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
