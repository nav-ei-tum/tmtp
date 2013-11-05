#ifndef TestProtConf_h
#define TestProtConf_h

#include "NetProtConf.h"
#include <stdint.h>

/*! \brief Test packet formatting (i.e. network protocol configuration). 
 * 
 * The functions here defined (but not implemented) deal with basic packet formatting.
 * E. g.: Encapsulates data to generate a packet, creates a packet header containing the packet length, 
 * reads a header and determines if a packet is idle, etc. \n
 *
 * \warning
 * This class was mostly made for testing purposes and some member functions assume ALL packets are idle! \n
 * In some cases the values passed to these functions are merely hardcoded constants.
 *
 */
 
class TestProtConf : public NetProtConf {			// Inherits from NetProtConf all its members as public.

// definitions
protected:
	static const uint16_t idlePacketVersion = 0;	/**< Predefined Packet Version pattern that indicates an Idle Packet */
	static const uint16_t testPacketVersion = 2;	/**< Predefined Packet Version pattern that indicates a Test Packet */
	static const uint8_t idlePacket = 0x1F;		/**< Predefined contents of an Idle Packet (0001 1111 in binary). */
	static const uint16_t packetHeaderLength = 2;	/**< Packet header length in Bytes. */

// methods
public:

/*! \brief Constructor for the Test Protocol Configurator class. */
	TestProtConf();

/*! \brief Takes first Byte of a packet header and checks if it contains an idle packet version.
 * \param firstByteOfHeader The first Byte of a packet header.
 * 
 * This function extracts the three most significant bits of the packet header (i.e. the packet version) and 
 * checks if they describe an idle packet by comparing them to the idlePacketVersion.
 */
	virtual bool isIdlePacket(uint8_t firstByteOfHeader);

/*! \brief Extracts and calculates the total packet length (header length + message length + 1).
 * \param header The whole packet header (2 bytes long).
 * 
 * Extracts the 13 least significant bits of the packet header (which contains the packet length).
 */
	virtual uint64_t extractPacketLength(vector<uint8_t> header);

/*! \brief Retrieves the hardcoded value of variable packetHeaderLength. 
 * \param firstByteOfHeader Supposed to be the first Byte of a packet header, but does absolutely nothing with it.
 * 
 * Intended to calculate the packet header length based on the contents of the 1st header Byte. However, this member function just retrieves the value of a hardcoded variable. \n
 * This function probably serves as a fail-safe mechanism in case a child class did not implement this function properly.
 *
 * \warning
 * Member function NOT IMPLEMENTED! Does absolutely nothing with the received parameters.
 *
 */
	virtual uint64_t getPacketHeaderLength(uint8_t firstByteOfHeader);

/*! \brief Generates an idle packet for testing purposes.
 * 
 * Intended to assemble a packet with idle contents. However, this member function just retrieves the value of a hardcoded variable. \n
 * This function probably serves as a fail-safe mechanism in case a child class did not implement this function properly.
 *
 * \warning
 * Does not generate any packet! It only retrieves the value of variable idlePacket (harcoded to 0x1F, 0001 1111 in binary).
 * 
 */
	virtual uint8_t genIdlePacket();

/*! \brief  Appends a 2-Byte header to a message (upper-layer data) to generate a space packet for testing.
 * \param message Upper-layer data to encapsulate.
 * 
 * The header is two Bytes long. The first three bits contain the testPacketVersion (hardcoded to 010)
 * The rest of the header contains packetHeaderLength (harcoded to 2) + message.size. 
 * 
 * \note
 * The total packet length value is truncated to 13 bits, therefore lengths beyond 8191 Bytes are NOT supported.
 *
 */	
	virtual vector<uint8_t> genTestPacket(vector<uint8_t> message);
	
/*! \brief Dissects a packet into its components and displays them as messages for debugging.
 * \param packet The test packet to dissect.
 * 
 * COMPONENTS OF THE PACKET HEADER AND PACKET CONTENTS:
 * A header consists of 2 Bytes that are appended to a message (upper-layer packet):
 *		 - The first three bits contain the packet version ("000" if idle, "010" for test).
 *		 - The remaining 13 bits contain the packet length (= message length + header length).
 *
 * The rest of the packet is the upper-layer data, which can be up to 8189 Bytes long.
 */
	virtual void packetDebugOutput(vector<uint8_t> packet);
};

#endif // TestProtConf_h
