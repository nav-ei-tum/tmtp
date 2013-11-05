#ifndef SpacePacketConf_h
#define SpacePacketConf_h

#include "NetProtConf.h"
#include <stdint.h>

/*! \brief This is polymorphic class implements some functions for "network protocol configuration". 
 * 
 * The functions here defined (some implemented) deal with packets.
 * E. g.: checks if a packet is idle, gets its length or assigns it a header, etc.
 *
 * \warning
 * This class was mostly made for testing purposes and some member functions assume ALL packets are idle! 
 * In some cases the values passed to these functions are merely hardcoded constants.
 *
 */

class SpacePacketConf : public NetProtConf {			// Inherits from NetProtConf all its members as public.

// definitions
protected:
	static const uint16_t idlePacketVersion = 1;	// 0000 0000 0000 0001
	static const uint16_t testPacketVersion = 0;	// 0000 0000 0000 0000
	static const uint8_t idlePacket = 0x20;		// 0010 0000
	static const uint16_t packetHeaderLength = 6;	// Six Bytes of header length.

// methods
public:

/*! \brief Constructor for the Space Packet Configurator class. */
	SpacePacketConf();

/*! \brief Takes first Byte of a packet header and checks if it contains an idle packet version.
 * \param firstByteOfHeader The first Byte of a packet header.
 * 
 * This function extracts the three most significant bits of the packet header (i.e. the packet version) and 
 * checks if they describe an idle packet by comparing them to the definition of the idle packet version.
 *
 * \warning
 * This member function was originally commented as being uncapable of detecting "real" idle packets. 
 * Asuming all idle packets have a packet version of 001, then the function should work just fine.
 *
 */
	virtual bool isIdlePacket(uint8_t firstByteOfHeader);

/*! \brief Extracts and calculates the total packet length (header length + message length + 1).
 * \param header The whole packet header (6 bytes long).
 * 
 * Extracts the 5th and 6th Bytes of the packet header (which contains the packet length) and adds them to the packetHeaderLength (hardcoded to 6). 
 * ... And then it adds a 1.
 */
	virtual uint64_t extractPacketLength(vector<uint8_t> header);

/*! \brief Returns the hardcoded value of variable packetHeaderLength.
 * \param firstByteOfHeader Supposed to be the first Byte of a packet header, but does absolutely nothing with it.
 * 
 * \warning
 * Memeber function NOT IMPLEMENTED! It only returns the value of protected variable packetHeaderLength.
 * Also, check the variable types! It returns a static const uint16_t as uint64_t!
 * Oh! And it also receives an uint8_t but does absolutely nothing with it.
 *
 */
	virtual uint64_t getPacketHeaderLength(uint8_t firstByteOfHeader);
	
/*! \brief Generates an idle packet for testing purposes.
 * 
 * \warning
 * This member function is NOT IMPLEMENTED! It only returns the value of idlePacket (harcoded to 0x20, 00100000 in binary) 
 *
 */
	virtual uint8_t genIdlePacket();
	
/*! \brief Appends a 6-Byte header to a message (upper-layer data) to generate a space packet for testing.
 * \param message Upper-layer data to encapsulate.
 * 
 * The 1st, 2nd and 4th Bytes are all zeros while the 3rd Byte stores a 192 (1100 0000), 
 * while the 5th and 6th contain the message length expressed as an uint16_t.
 */
	virtual vector<uint8_t> genTestPacket(vector<uint8_t> message);
	
/*! \brief Dissects a packet into its components and displays them as messages for debugging.
 * \param packet The space packet to dissect.
 * 
 * COMPONENTS OF THE PACKET HEADER AND PACKET CONTENTS:
 * A header consists of 6 Bytes that are appended to a message (upper-layer packet):
 * The 1st and 2nd Bytes constitute the packet ID:
 *		 - The first three bits contain the packet version ("001" if idle).
 *		 - The 4th bit contains the packet type.
 *		 - The 5th bit contains the Data Field Header Present Flag.
 *		 - The remaining 11 bits are called apid (application ID?).
 *
 * The 3rd and 4th Bytes are the packet sequence control:
 *		 - The first two bits contain the Groupings Flag.
 *		 - The remaining 14 bits contain the Source Sequence Counter.
 *
 * The 5th and 6th Bytes contain the total packet length (message length + packet heather length + 1)
 *
 * If the Data Field Header Present Flag = TRUE, then the components of the packet contents are:
 * The 2nd, 3rd and 4th bits of the 1st Byte of the message are called "pusVersion".
 * The 2nd Byte "pusServiceType", the 3rd "pusServiceSubtype and the 4th "pusPacketSubcounter".
 */
	virtual void packetDebugOutput(vector<uint8_t> packet);
};

#endif // SpacePacketConf_h
