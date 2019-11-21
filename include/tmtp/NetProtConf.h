#ifndef NetProtConf_h
#define NetProtConf_h

//////////////////////////////////////////////////////////////////////////////
// C++ Class NetProtConf                                                    //
// Author: Quirin Funke (TUM)                                               //
// Date: 10/11/2011                                                         //
// Project: EcssGateway                                                     //
// NetProtConf is the base class for network protocol configurations.       //
// It supports only idle packets. Derived classes can implement real        //
// protocols like CAN, IP or LunaNet.                                       //
//////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <stdint.h>

using namespace std;

/*! \brief Base class for packet formatting (i.e. network protocol configuration). 
 * 
 * The functions here defined (but not implemented) deal with basic packet formatting.
 * E. g.: Encapsulates data to generate a packet, creates a packet header containing the packet length, 
 * reads a header and determines if a packet is idle, etc. \n
 *
 *	\warning As this is just a base class for configurations, this class assumes ALL packets are idle! \n 
 * Derived classes can implement real protocols (e. g. CAN, IP or LunaNet)
 * this ensures that nothing bad can happen.
 */

class NetProtConf {

// methods
public:

	/*! \brief Constructor for the Network Protocol Configuration class.	*/
	NetProtConf();
	
	/*! \brief Receives a packet header (only the first 8 bits) and checks if it belongs to an idle packet.
	 * 
	 * This virtual member is NOT IMPLEMENTED. It simply returns TRUE every time.
	 */
	virtual bool isIdlePacket(uint8_t firstByteOfHeader);
	
	/*! \brief Receives the header of a packet (all 2 Bytes) and extracts the packet length (stored in the last 13 bits of the header).
	 * 
	 * This virtual member is NOT IMPLEMENTED. It simply returns a 1.
	 * Idle packets have a length of 1.
	 */
	virtual uint64_t extractPacketLength(vector<uint8_t> header);

	/*! \brief Receives the header of a packet (only the first 8 bits) and returns its length.
	 * 
	 * This virtual member is NOT IMPLEMENTED. It simply returns a 1.
	 * Idle packets have a length of 1 and contain only a header.
	 */
	virtual uint64_t getPacketHeaderLength(uint8_t firstByteOfHeader);

	/*! \brief generates an "idle packet" by returning just an asterisk "*".
	 * 
	 * This virtual member is NOT IMPLEMENTED. 
	 * It simply returns an "*", because the actual idle packet content is unimportant.
	 */
	virtual uint8_t genIdlePacket();

	/*! \brief Receives a message vector and returns it with a header with hard-coded test values.
	 * 
	 * This virtual member is NOT IMPLEMENTED. It simply returns a 1.
	 */
	virtual vector<uint8_t> genTestPacket(vector<uint8_t> message);

	/*! \brief Displays a message (for debugging purposes) with the packet contents and its size in decimal notation.
	 * 
	 * This member assumes all packets are idle packets! It displays: IdlePacket[SIZE_IN_BITS] Content: "PACKET_CONTENT_AS_IS
	 */
	virtual void packetDebugOutput(vector<uint8_t> packet);
};

#endif // NetProtConf_h
