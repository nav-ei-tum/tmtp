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
