#ifndef PacketServer_h
#define PacketServer_h

class TmVirtualChannel;	// // Uses the Virtual Channel class.
class NetProtConf;		// Uses the NetProtConf class ... Oh noes!

/*!	\brief Specifies the Virtual Channel and Network Protocol Configuration used. */
class PacketServer
{
//
// methods
//
public:

/*!	\brief Constructor of the PacketServer class.
	\param conf Set of functions that define how the packet will be handled (i.e. the network protocol).
*/
	PacketServer(NetProtConf *conf);

/*!	\brief Establishes the Virtual Channel these packet will belong.
	\param vc Virtual Channel through which these packets will be sent.
*/
	virtual void connectTmVc(TmVirtualChannel *vc);
	
/*!	\brief Sets the Virtual Channel to NULL. */
	virtual void disconnectTmVc();

	//virtual void connectTcVc(TcVirtualChannel *vc);
	//virtual void disconnectTcVc();

//
// variables
//
protected:
	NetProtConf *netProtConf;		/*!< Set of functions that define how the packet will be handled (i.e. the network protocol). */
	TmVirtualChannel *tmVc;			/*!< Virtual Channel through which these packets will be sent. */
	//TcVirtualChannel *tcVc;
	//TODOn TcpServer;
};

#endif // PacketServer_h
