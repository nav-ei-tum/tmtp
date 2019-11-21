#ifndef TmFrameBitrate_h
#define TmFrameBitrate_h

#include <stdint.h>

using namespace std;
/*!	\brief Offers a way to store the transmission bit rate in <b>bits per second</b> within a TmTrasferFrame object.\n
 *
 * Its purpose is to allow a way to estimate the time at which the first bit of a packet (within the frame) was transmitted. 
 * This information is used as a packet timestamp.
 * The bitrate is not part of the TMTP protocol, it is therefore completely optional.\n
 *
 * The way to implement HOW to calculate the actual bitrate is left to the lower layers. 
 *\note This particular bitrate is meant to be stored in the frame <b> at the receiving end</b>. Not upon transmission.
 */
class TmFrameBitrate {

//
// definitions
//
protected:
	static const double bpsInitPattern;		/*!< Initialization pattern for the bitrate. While stored in variable bps, means no bitrate has been provided. */
	
//
// methods
//
	public:
	
/*! \brief Constructor of the Bitrate class. */
		TmFrameBitrate();

/*! \brief Sets the bitrate in bits per second.
 *	\param bps bitrate in bps.
 */
		virtual void setBitrate(double rate);

/*! \brief Retrieves the bitrate.*/
		virtual double getBitrate();

/*! \brief Shows if a given bitrate has actual data.
 *	\todo Think of a more clever way to verify a bitrate is valid. Perhaps using a -1.0?.
 *
 * The isValid() method is intended for the case that the receiver application using TMTP does not provide a bitrate. 
 * 
 * The idea is to consider a TmFrameBitrate object "empty" if isValid() returns FALSE. 
 * This way, any function in the library receiving a bitrate object will always get one, thus avoiding NULL pointer verification, usage of overloaded functions, etc.
 * 
 * Its operation is simple: The variable bps is initialized with a hex pattern of 0x0F0F0F0F0F0F0F0F (aproximately 1.0851e+018 in decimal). 
 * After actual bitrate data has been assigned to bps, the pattern will change to something else. This function verifies bps does not have the initialization pattern.
 *
 *	\note The initialization pattern for bps will remain true for both 32- and 64-bit architectures. 
 * C++ follows the IEEE 754 double-precision floating point standard, and such numbers are always 64 bits.
 */
		virtual bool isValid();

//
// variables
//
	protected:
		double bps;	/*!< The bitrate in bits per second.*/

};

#endif // TmFrameBitrate_h
