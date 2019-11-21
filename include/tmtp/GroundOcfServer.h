#ifndef GroundOcfServer_h
#define GroundOcfServer_h

#include "OcfServer.h"
#include "myErrors.h"

/*! \brief Implements the server functions for a ground station OCF application. */
class GroundOcfServer : public OcfServer
{
//
// methods
//
public:	

/*! \brief Constructor of the Ground OCF Server class. 
 *
 * Sets the debug output flag to FALSE.
 */
	GroundOcfServer();

/*! \brief Retrieves the 1st OCF message in the reception queue and optionally displays the corresponding debug messages.
 *
 * If this OCF Server has been connected to a master channel, and while there are any (received) OCF messages in the OCF input queue, 
 * Retrieves the 1st OCF message in the input queue. If the debugOutput flag is TRUE, displays the corresponding debug messages.
 *
 * \note A GroundOcfServerError will be thrown if:
 *	- No master channel has been connected to this OCF Server.
 *	- Any error is generated while creating the OCF object.
 */	
	virtual void signalNewOcf();

/*! \brief Sets the debugOutput flag to TRUE. */	
	virtual void activateDebugOutput();

/*! \brief Sets the debugOutput flag to FALSE. */	
	virtual void deactivateDebugOutput();

//
// variables
//
protected:
	bool debugOutput;	/*!< Indicates whether any debug messages are to be displayed or not (default = FALSE). */
};

#endif // GroundOcfServer_h
