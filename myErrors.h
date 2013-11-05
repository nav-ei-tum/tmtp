#ifndef myErrors_h
#define myErrors_h

#include <stdexcept>
#include <string>
#include <stdint.h>

using namespace std;

//
// Definition of error objects of other classes.
// They all inherit from std::runtime_error.
//

/*! \brief Reports any errors related to the creation of TM Transfer Frames.
 *
 * Inherits the contructor of std::runtime_error. \n
 * Basically, this is just runtime_error under another name. 
 * Each time something bad happens within the TmTransferFrame class (e.g. a variable exceeds the expected boundaries) 
 * there is a "throw" instruction specifying what went wrong using a message stored in a string variable. \n
 * Any error messages or warnings triggered are collected in a myErrors::TmChannelWarning object (using a try-catch structure).
 */
class TmTransferFrameError : public runtime_error {
public:

/*! \brief Constructor of the TmTransferFrameError class.
 *	\param what_arg The error message to display or to accumulate.
 */
	explicit TmTransferFrameError(const string& what_arg)
		: runtime_error(what_arg)
	{}
};

/*! \brief Reports any errors related to the Operational Control Field.
 *
 * Inherits the contructor of std::runtime_error. \n
 * Basically, this is just runtime_error under another name. 
 * Each time something bad happens within the TmOcf class (e.g. a variable exceeds the expected boundaries) 
 * there is a "throw" instruction specifying what went wrong using a message stored in a string variable. \n
 * Any error messages or warnings triggered are collected in a myErrors::TmChannelWarning object (using a try-catch structure).
 */
class TmOcfError : public runtime_error {
public:

/*! \brief Constructor of the TmOcfError class.
 *	\param what_arg The error message to display or to accumulate.
 */ 
	explicit TmOcfError(const string& what_arg)
		: runtime_error(what_arg)
	{}
};

/*! \brief Reports any errors related to the virtual channels.
 *
 * Inherits the contructor of std::runtime_error. \n
 * Basically, this is just runtime_error under another name. 
 * Each time something bad happens within the TmVirtualChannel class (e.g. a variable exceeds the expected boundaries) 
 * there is a "throw" instruction specifying what went wrong using a message stored in a string variable. \n
 * Any error messages or warnings triggered are collected in a myErrors::TmChannelWarning object (using a try-catch structure).
 */
class TmVirtualChannelError : public runtime_error {
public:

/*! \brief Constructor of the TmVirtualChannelError.
 *	\param what_arg The error message to display or to accumulate.
 *	\param id The virtual channel ID where the error(s) originated.
 */
	explicit TmVirtualChannelError(uint16_t id, const string& what_arg)
		: runtime_error(what_arg)
	{ vcid = id; }

/*! \brief Retrieves the virtual channel ID where the error(s) originated. */
	virtual uint16_t getVcid()
	{ return vcid; } 
private:
	uint16_t vcid; /**< Virtual channel ID where the error(s) originated. */
};

/*! \brief Reports any errors related to the master channels.
 *
 * Inherits the contructor of std::runtime_error. \n
 * Basically, this is just runtime_error under another name. 
 * Each time something bad happens within the TmMasterChannel class (e.g. a variable exceeds the expected boundaries) 
 * there is a "throw" instruction specifying what went wrong using a message stored in a string variable. \n
 * Any error messages or warnings triggered are collected in a myErrors::TmChannelWarning object (using a try-catch structure).
 */
class TmMasterChannelError : public runtime_error {
public:

/*! \brief Constructor of the TmMasterChannelError class.
 *	\param what_arg The error message to display or to accumulate.
 */
	explicit TmMasterChannelError(const string& what_arg)
		: runtime_error(what_arg)
	{}
};

/*! \brief Reports any errors related to the physical channel.
 *
 * Inherits the contructor of std::runtime_error. \n
 * Basically, this is just runtime_error under another name. 
 * Each time something bad happens within the TmPhysicalChannel class (e.g. a variable exceeds the expected boundaries) 
 * there is a "throw" instruction specifying what went wrong using a message stored in a string variable. \n
 * Any error messages or warnings triggered are collected in a myErrors::TmChannelWarning object (using a try-catch structure).
 */
class TmPhysicalChannelError : public runtime_error {
public:

/*! \brief Constructor of the TmPhysicalChannelError class.
 *	\param what_arg The error message to display or to accumulate.
 */
	explicit TmPhysicalChannelError(const string& what_arg)
		: runtime_error(what_arg)
	{}
};

/*! \brief Reports any errors related to the ground packet server.
 *
 * Inherits the contructor of std::runtime_error. \n
 * Basically, this is just runtime_error under another name. 
 * Each time something bad happens within the GroundPacketServer class (e.g. a file could not be opened) 
 * there is a "throw" instruction specifying what went wrong using a message stored in a string variable. \n
 */
class GroundPacketServerError : public runtime_error {
public:

/*! \brief Constructor of the GroundPacketServerError class.
 *	\param what_arg The error message to display or to accumulate.
 */
	explicit GroundPacketServerError(const string& what_arg)
		: runtime_error(what_arg)
	{}
};

/*! \brief Reports any errors related to the ground OCF server.
 *
 * Inherits the contructor of std::runtime_error. \n
 * Basically, this is just runtime_error under another name. 
 * Each time something bad happens within the GroundOcfServerError class (e.g. a file could not be opened) 
 * there is a "throw" instruction specifying what went wrong using a message stored in a string variable. \n
 */
class GroundOcfServerError : public runtime_error {
public:

/*! \brief Constructor of the GroundOcfServerError class.
 *	\param what_arg The error message to display or to accumulate.
 */
	explicit GroundOcfServerError(const string& what_arg)
		: runtime_error(what_arg)
	{}
};

/*! \brief Reports any errors related to the reception of a timestamp.
 *
 * Inherits the contructor of std::runtime_error. \n
 * Basically, this is just runtime_error under another name. 
 * Each time any of the timestamp variables exceeds the expected boundaries there is a "throw" instruction 
 * specifying what went wrong using a message stored in a string variable. \n
 */
class TmFrameTimestampError : public runtime_error {
public:

/*! \brief Constructor of the TmFrameTimestampError class.
 *	\param what_arg The error message to display or to accumulate.
 */
	explicit TmFrameTimestampError(const string& what_arg)
		: runtime_error(what_arg)
	{}
};

/*! \brief Reports any errors related to the reception of a bitrate.
 *
 * Inherits the contructor of std::runtime_error. \n
 * Basically, this is just runtime_error under another name. 
 * Each time the received bitrate exceeds the expected boundaries there is a "throw" instruction 
 * specifying what went wrong using a message stored in a string variable. \n
 */
class TmFrameBitrateError : public runtime_error {
public:

/*! \brief Constructor of the BitrateError class.
 *	\param what_arg The error message to display or to accumulate.
 */
	explicit TmFrameBitrateError(const string& what_arg)
		: runtime_error(what_arg)
	{}
};


//
// warning objects
//

/*! \brief Processes all warning and error messages generated at any point in the transmission or reception of TMTP-encapsulated packets. */
class TmChannelWarning {
public:

/*! \brief Constructor for the Channel Warnings class. 
 *
 * Sets the following default values:
 * 	- frameUnwrapError is cleared.
 * 	- lostMCFrames = 0
 * 	- lostVCFrames = 0
 * 	- packetResync = false
 * 	- noPacketSinkSpecified = false
 * 	- noOcfSinkSpecified = false
 * 	- unconfiguredVC = false
 * 	- unconfiguredMC = false
 * 	- recPacketBufferOverflow = false
 * 	- recOcfBufferOverflow = false
 * 	- wrongOcfFlag = false
 * 	- wrongScid = false
 * 	- wrongVcid = false
 * 	- wrongSecondHeaderFlag = false
 * 	- wrongSynchronisationFlag = false
 * 	- freeMessage is cleared.
 */
	TmChannelWarning();

/*! \brief Overloading operator function for "+=" accumulates error messages.
 *	\param rhs The "right hand side" copies of the variables to accumulate.
 *
 * The meaning of "C += X" is "C = C + X". The purpose of this member function is to implement this functionality to add strings variables and 
 * to OR boolean variables with themselves, i.e. accumulate them:
 *	- String and integer errors are accumulated with "+=". Example: frameUnwrapError += rhs.frameUnwrapError. \n
 *	- Boolean errors are OR'd - Each type of error needs to happen just once to be marked as present. Example: wrongOcfFlag |= rhs.wrongOcfFlag. \n
 *
 * For instance, lets say frameUnwrapError encountered a problem: "Direct data field access configured but corresponding receive function pointer not connected." \n
 * ... Later, in another frame, the following warning was thrown: "Direct data field access function return data field of wrong size."\n
 * Both would be added by TmChannelWarning::addFrameUnwrapError(string msg) and stored in frameUnwrapError in the following way: \n
 * frameUnwrapError += msg; \n
 * frameUnwrapError += "; "; \n
 * At the end, that function would produce something like: \n
 * "Direct data field access configured but corresponding receive function pointer not connected.; Direct data field access function return data field of wrong size."
 */
	virtual TmChannelWarning & operator+=(const TmChannelWarning &rhs);

/*! \brief Removes newlines, replaces them with blank spaces, appends current msg into frameUnwrapError and adds a semicolon at its end.
 *	\param msg The error/warning message to store and accumulate in the frameUnwrapError variable.
 */
	virtual void addFrameUnwrapError(string msg);

/*! \brief Accumulates the total ammount of master channel lost frames.
 *	\param count The ammount of lost frames at the end of a single scan.
 */
	virtual void addMCLostFramesCount(uint16_t count);

/*! \brief Accumulates the total ammount of virtual channel lost frames.
 *	\param count The ammount of lost frames at the end of a single scan.
 */
	virtual void addVCLostFramesCount(uint64_t count);

/*! \brief Sets the packetResync flag to TRUE. */
	virtual void setPacketResynced();

/*! \brief Sets the noPacketSinkSpecified flag to TRUE. */
	virtual void setNoPacketSinkSpecified();

/*! \brief Sets the noOcfSinkSpecified flag to TRUE. */
	virtual void setNoOcfSinkSpecified();

/*! \brief Sets the unconfiguredVC flag to TRUE. */
	virtual void setUnconfiguredVC();

/*! \brief Sets the unconfiguredMC flag to TRUE. */
	virtual void setUnconfiguredMC();

/*! \brief Sets the recPacketBufferOverflow flag to TRUE. */
	virtual void setRecPacketBufferOverflow();

/*! \brief Sets the recOcfBufferOverflow flag to TRUE. */
	virtual void setRecOcfBufferOverflow();

/*! \brief Sets the wrongOcfFlag flag to TRUE. */
	virtual void setWrongOcfFlag();

/*! \brief Sets the wrongScid flag to TRUE. */
	virtual void setWrongScid();

/*! \brief Sets the wrongVcid flag to TRUE. */
	virtual void setWrongVcid();

/*! \brief Sets the wrongSecondHeaderFlag flag to TRUE. */
	virtual void setWrongSecondHeaderFlag();

/*! \brief Sets the wrongSynchronisationFlag flag to TRUE. */
	virtual void setWrongSynchronisationFlag();

/*! \brief Removes newlines, replaces them with blank spaces, appends current msg into FreeMessage and adds a semicolon at its end.
 *	\param msg The error/warning message to store and accumulate in the FreeMessage variable.
 */
	virtual void appendFreeMessage(string msg);

/*! \brief Retrieves any and all errors and warnings stored and returns them as a single message - All non-empty error variables are then cleared. */
	virtual string popWarning();

/*! \brief Checks if at least one of the possible warning causes has thrown a warning. */
	virtual bool warningAvailable();

//
// variables
//	
private:
	string frameUnwrapError;		/*!< Error message while unwrapping a frame. */
	uint16_t lostMCFrames;	/*!< Ammount of lost frames in a master channel. */
	uint64_t lostVCFrames;		/*!< Ammount of lost frames in a vitual channel. */
	bool packetResync;				/*!< Indicates if a packet has been moved within the Data Field. */
	bool noPacketSinkSpecified;		/*!< Indicates if a packet sink has been defined in the ground packet server. */
	bool noOcfSinkSpecified;		/*!< Indicates if an OCF sink has been defined in the ground OCF server. */
	bool unconfiguredVC;			/*!< Indicates if a virtual channel has been defined for the current frame. */
	bool unconfiguredMC;			/*!< Indicates if a master channel has been defined for the current frame. */
	bool recPacketBufferOverflow;	/*!< Indicates a buffer overflow in the recieved packet buffer. */
	bool recOcfBufferOverflow;		/*!< Indicates a buffer overflow in the recieved OCF buffer. */
	bool wrongOcfFlag;				/*!< Indicates the OCF flag in the received frame and in the channel settings don't match. */
	bool wrongScid;					/*!< Indicates the Spacecraft ID in the received frame and in the master channel settings don't match. */
	bool wrongVcid;					/*!< Indicates the virtual channel ID of the received frame does not match its virtual channel settings. */
	bool wrongSecondHeaderFlag;		/*!< Indicates the Secondary Header Flag of the received frame does not match its virtual channel settings. */
	bool wrongSynchronisationFlag;	/*!< Indicates the Secondary Header Flag of the received frame does not match its virtual channel settings. */
	string freeMessage;				/*!< Accumulates warning messages. */
};

#endif // myErrors_h
