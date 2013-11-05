#ifndef GroundPacketServer_h
#define GroundPacketServer_h

#include "PacketServer.h"
#include "myErrors.h"

class NetProtConf;	// Uses NetProtConf as base class to define the object type used for packet configuration.

/*!	\brief Specifies the Virtual Channel and Network Protocol Configuration used. */
class GroundPacketServer : public PacketServer
{
//
// methods
//
public:	

/*! \brief Constructor of the GroundPacketServer class inherits from the PacketServer class.
 *	\param conf Set of functions that define how the packet will be handled (i.e. the network protocol).
 *
 * By default, debug output messages are deactivated.
 */
	GroundPacketServer(NetProtConf *conf);

/*! \brief Retrieves all packets in the VC input queue and stores each of them in a separate binary file under "outputFiles/".
 *
 * If a virtual channel has been defined and while there are packets waiting in the input queue:
 *	- Retrieves a packet from the input queue and shows its contents (if debug output activated).
 *	- Creates a string (to be used as a path) with the creation time: "outputFiles/<Creation_Time>".
 *	- Creates a binary file with time of creation as the file name under the outputFiles folder.
 *	- Stores the contents of the packet in the binary file.
 *
 *	\note may throw GroundPacketServerError if:
 *	- Failed to open/write to the binery file.
 *	- No virtual channel was specified.
 *	- There were any virtual channel errors.
 */
	virtual void signalNewPacket();

/*! \brief Sets the debug output flag to TRUE.. */
	virtual void activateDebugOutput();

/*! \brief Sets the debug output flag to FALSE. */
	virtual void deactivateDebugOutput();

//
// variables
//
protected:
	bool debugOutput;	/*!< Indicates whether any debug messages are to be displayed or not (default = FALSE). */
};

#endif // GroundPacketServer_h
