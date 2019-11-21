#ifndef TmFrameTimestamp_h
#define TmFrameTimestamp_h

#include "myErrors.h"
#include <stdint.h>

using namespace std;

/*!	\brief Offers a way to store seconds and fractions of seconds within a TmTrasferFrame object.
 * 
 * Its purpose is to allow a way to use timestamps with the TMTP protocol.
 * However, these timestamps are not part of the TMTP protocol, therefore are completely optional.\n
 *
 * The way to implement HOW to measure time at the moment of storing it as a timestamp is left 
 * to the application using this TMTP Library. \n 
 *
 * The amount of seconds must be counted after Epoch (1st of January, 1970).
 *\note This particular timestamp is meant to be stored in the frame <b> at the receiving end</b>. Not upon transmission.
 */
class TmFrameTimestamp 
{

//
// methods
//
public:
	
/*! \brief Constructor of the Timestamp class.*/
	TmFrameTimestamp();

/*! \brief Sets the seconds part of the timestamp. 
 *	\param secs The seconds part of the timestamp.
 *
 * Seconds stored must be an unsigned integer smaller than 2^64.
 * These should express the amount of seconds since Epoch up until the time and date of the timestamp.
 */
	virtual void setSeconds(uint64_t secs);

/*! \brief Sets the fractions of seconds of the timestamp.
 *	\param fracs The fractions of seconds of the timestamp. 
 *	\note Fractions must be smaller than 1 and greater than or equal to zero
 */
	virtual void setFractions(double fracs);

/*! \brief Retrieves the seconds part of the timestamp.*/
	virtual uint64_t getSeconds();

/*! \brief Retrieves the fractions of seconds of the timestamp. */
	virtual double getFractions();

/*! \brief Shows if a given timestamp has actual data measuring the amount of seconds after Epoch.
 *
 * The isValid() method is intended for the case that the receiver application using TMTP does not provide a timestamp. 
 * 
 * The idea is to consider a TmFrameTimestamp object "empty" if isValid() returns FALSE. 
 * This way, any function in the library receiving a timestamp object will always get one, thus avoiding NULL pointer verification, usage of overloaded functions, etc.
 * 
 * Its operation is simple: A timestamp counts the seconds ellapsed since Epoch. 
 * Therefore, a variable "seconds" containing zero (the initialization value), means no timestamp was provided. 
 */
	virtual bool isValid();
		
//
// variables
//
protected:
		uint64_t seconds;	/*!< The seconds part of the timestamp.*/
		double fractions;	/*!< The fractions of a second of the timestamp.*/

};

#endif // TmFrameTimestamp_h
