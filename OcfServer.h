#ifndef OcfServer_h
#define OcfServer_h

class TmMasterChannel;	// Uses the TmMasterChannel class.


/*! \brief Implements a server application base class for establishing and breaking connections to send and receive OCF messages. */
class OcfServer
{
//
// methods
//
public:

/*! \brief Constructor of the OcfServer class.
 *
 * Sets the tmMc (master channel) pointer to NULL.
 */
	OcfServer();

/*! \brief Specifies the master channel to which this OCF Server will be linked.
 *	\param mc The pointer to the master channel to connect to.
 */	
	virtual void connectTmMc(TmMasterChannel *mc);

/*! \brief Sets the master channel pointer to NULL. */	
	virtual void disconnectTmMc();

	//virtual void connectTcMc(TcMasterChannel *mc);
	//virtual void disconnectTcMc();

//
// variables
//
protected:
	TmMasterChannel *tmMc;		/*!< Pointer to a Master Channel. */
	//TcMasterChannel *tcMc;
	//TODOn TcpServer;
};

#endif // OcfServer_h
