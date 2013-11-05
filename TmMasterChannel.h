#ifndef TmMasterChannel_h
#define TmMasterChannel_h

#include "TmOcf.h"
#include "myErrors.h"
#include "ticp/TicpTimestamp.hpp"

#include <queue>
#include <vector>
#include <stdint.h>

using namespace std;

// Uses the following classes:
class TmTransferFrame;
class TmPhysicalChannel;
class TmVirtualChannel;
class GroundOcfServer;

/*! \brief Implementation of a TMTP Master Channel.
 *
 * According to the ECSS-E-ST-50-03C standard, the master channel consists of all TM Transfer frames that share the same TF Version Number and Spacecraft ID. \n
 * The key aspects to consider of TMTP connections are:
 *	- A physical channel is a connection to a spacecraft and has at least one master channel.
 *	- A physical channel can be divided into multiple master channels (not implemented).
 *	- A master channel is divided into a maximum of eight virtual channels.
 *	- Virtual channels enable one physical channel to be shared among multiple higher-layer data streams.
 * 
 * For a typical space mission, all the frames on a physical channel have the same Spacecraft ID. Therefore there is typically only one master channel per physical channel. \n
 * However, the standard specifies that multiple master channels can share a physical channel. This can be the case when one spacecraft is transporting another spacecraft such as a probe. 
 * This feature is currently not implemented.
 */
class TmMasterChannel
{
//
// definitions
//
protected:
	static const uint16_t recOcfBufferSize = 100;		/*!< Maximum ammount of received OCF messages waiting for processing. */
	static const uint16_t sendOcfBufferSize = 100;	/*!< Maximum ammount of OCF messages waiting to be sent. */

//
// methods
//
public:

/*! \brief Constructor of the TmMasterChannel class.
 *	\param scid The Spacecraft ID for this master Channel.
 *	\param *parent The physical channel to which this master channel belongs.
 * 
 * Creates a master channel with the following (default) attributes:
 * 		- ocfPresent = true;
 * 		- sendFrameCount = 0;
 * 		- recFrameCount = 0;
 * 		- secondHeaderPresent = false;
 * 		- extendedVcFrameCountUsed = false;
 * 		- ocfSink = NULL;
 * 		- currentVc = 0;
 * 		- idleChannel = 7;
 *
 * Links this master channel with a physical channel through the *parent pointer. \n
 * Creates a vector of eight virtual channels. All, except the last one, are empty (they point to NULL). \n
 * The last one is considered as idle and it is created with the following (default) attributes:
 *		- virtualChannelId = 7;
 *		- packetSink = NULL;
 *		- secondHeaderPresent = false;
 *		- extendedFrameCountSet = false;
 *		- dataFieldSynchronised = true;
 *		- initialConf = netProtConf = new NetProtConf;
 *		- debugOutput = false;
 *		- sendFrameCount = 0;
 *		- recFrameCount = 0;
 *		- recPacketHeaderLength = 0;
 *		- recPacketLength = 0;
 * 
 * \note A TmMasterChannelError is thrown if:
 *	- The Spacecraft ID does not fall within the range of 0 to 1023.
 *	- The idle Virtual Channel ID does not fall within the range of 0 to 7.
 */
	TmMasterChannel(uint16_t scid, TmPhysicalChannel *parent); 
	
/*! \brief Destructor of the TmMasterChannel class.
 *
 * Removes all eight of the generated VCs from memory.
 */
	~TmMasterChannel();

/*! \brief Retrieves the Spacecraft ID. */
	virtual uint16_t getSpacecraftId();

/*! \brief Retrieves the sent frame counter. */
	virtual uint16_t getSendFrameCount();

/*! \brief Retrieves the received frame counter. */
	virtual uint16_t getRecFrameCount();

/*! \brief Retrieves the total frame length as configured in the physical channel. */
	virtual uint16_t getFrameLength();

/*! \brief Retrieves the FECF flag as configured in the physical channel. */
	virtual bool getFecfStatus();

/*! \brief Sets the OCF Flag to TRUE. */
	virtual void activateOcf();

/*! \brief Sets the OCF Flag to FALSE. */
	virtual void deactivateOcf();

/*! \brief Retrieves the value of the OCF Flag. */
	virtual bool getOcfStatus();

/*! \brief Places an OCF message in an output Queue for transmission, iff the OCF Queue has not reached its limit.
 *	\param ocf The OCF instance to place in the output queue.
 *
 * \note May throw TmMasterChannelError.
 */
	virtual void sendOcf(TmOcf ocf);

/*! \brief Retrieves the 1st OCF message in the input queue, if any.
 * 
 * \note May throw TmMasterChannelError.
 */
	virtual TmOcf receiveOcf();

/*! \brief Returns TRUE if there is any OCF message in the OCF input queue. */
	virtual bool ocfAvailable();

/*! \brief Establishes an whole channel as idle (default is 7).
 *	\param channel The new channel to be marked as idle.
 *
 * By default, the last channel will carry only idle data, thus becoming the idle channel. If desired, the idle channel can be any other channel of the 8 available. \n
 * \note May throw TmMasterChannelError.
 */
	virtual void setIdleChannel(uint16_t channel);

/*! \brief Retrieves the current idle channel. */
	virtual uint16_t getIdleChannel();

/*! \brief Extracts the contents of the idle channel. */
	virtual TmVirtualChannel* getIdleChannelObject();

/*! \brief Checks each virtual channel and indicates whether there is at least one of them with frames to send. */
	virtual bool frameAvailable();

/*! \brief Verifies if a received frame has the correct settings for this master channel.
 *	\param frame The received frame to analyze.
 *	\return An instance of TmChannelWarning with any warnings/errors occured.
 * 
 * Analyzes the contents of the received frame and verifies the following matches the physical channel settings:
 *	- The Spacecraft ID.
 *	- Master Channel Frame Counter (if they don't match, they local MC counter is rectified).
 *	- OCF Flag.
 * Also, the position in the VC vector corresponding to the VC ID received is checked for configuration. \n
 * Lastly, if the input OCF queue has not reached its limit, it extracts the OCF message and puts it in the input queue.
 */
	virtual TmChannelWarning receiveFrame(TmTransferFrame frame);

/*! \brief Prepares a frame according to the physical channel settings to be sent over a virtual channel and appends a timestamp.
 *	\param timestamp A Timestamp as specified in the TICP namespace.
 * 
 * Creates a TM Transfer Frame with the minimum size and redimentions it to the size configured in the physical channel. \n
 * Using a Round Robin scheduling cycles through all of the channels looking for a VC with a frame available to be sent. Once found, prepares the frame to be sent. \n
 * Only unconfigured VCs and the idle channel are skipped. \n
 * Verifies the frame OCF settings match the master channel OCF settings. If OCF Flag = TRUE, takes an OCF message from the output queue and inserts it in the frame. \n
 * Copies the Spacecraft ID and Master Channel Frame Counter of the master channel into the VC.
 * Increases the Master Channel Frame Counter and returns the prepared frame.
 *
 * \todo If the output OCF queue IS empty, it is not defined what to do. Perhaps send some sort of "idle OCF" using the "Reserved for Future Use" report type?
 *
 * \note May throw TmMasterChannelError.
 */
	virtual TmTransferFrame sendFrame(ticp::Timestamp timestamp);

/*! \brief Establishes the sink where OCF messages are received (GroundOcfServer instance).
 * \param sink Pointer to the OCF Sink to which this channel has been connected.
 */
	virtual void connectOcfSink(GroundOcfServer *sink);

/*! \brief Sets the OCF sink pointer to NULL. */
	virtual void disconnectOcfSink();

/*! \brief Creates a new VC instance with default values and places a new pointer to it in the VC vector.
 *	\param vcid ID of the new virtual channel.
 *
 * Creates a new virtual channel pointer initialized to NULL.
 * If the specified VC ID is within boundaries (0-7), creates a pointer with the new VC instance linked to the current master channel.
 * If there is already a pointer in the VC vector in the specified VC ID, it is removed. Otherwise, the new pointer is placed in that position.
 *
 * \note May throw TmMasterChannelError if:
 *	- The VC ID is out of boundaries.
 *	- There was an error creating the new VC.
 *	- The specified VC vector position (vcid) is the idle channel.
 */
	virtual TmVirtualChannel* createTmVirtualChannel(uint16_t vcid);

/*! \brief Removes the specified virtual channel from the VC vector and sets its contents to point to NULL.
 *	\param vcid ID of the virtual channel to remove.
 */
	virtual void deleteTmVirtualChannel(uint16_t vcid);

protected:

/*! \brief Retrieves the 1st OCF message in the OCF sink reception queue (if any) and displays the corresponding debug messages (optional).
 *
 * Creates and returns a TmChannelWarning instance with any warnings or error messages generated.
 * If an OCF sink is defined (instance of GroundOcfServer), retrieves the 1st message in the input queue of the OCF sink 
 * and optionally displays debug messages.
 */
	virtual TmChannelWarning signalNewOcf();

//
// variables
//
protected:
	TmPhysicalChannel *physicalChannel;				/*!< The physical channel to which this master channel belongs (its pointer).*/
	vector<TmVirtualChannel*> virtualChannels;		/*!< Group of virtual channels (their pointers) for the current master channel. */
	GroundOcfServer *ocfSink;						/*!< Pointer to an instance of Ground OCF Server class which will receive OCF messages. */
	queue<TmOcf> sendOcfFifo;						/*!< Queue of OCF elements to send. */
	queue<TmOcf> recOcfFifo;						/*!< Queue of received OCF elements. */
	uint16_t currentVc;						/*!< The virtual circuit currently working on. */
	
	// attributes set for this master channel
    uint16_t spacecraftId;					/*!< Spacecraft Identifier. */
    bool ocfPresent;								/*!< Operational Control Field Flag. */
    uint16_t sendFrameCount;					/*!< Sent frame counter. */
    uint16_t recFrameCount;					/*!< Received frame counter. */
    bool secondHeaderPresent;						/*!< Secondary Header Flag. */
	bool extendedVcFrameCountUsed;					/*!< (Not part of the standard) Locally used flag to indicate usage of the Extended VC Frame Counter. */
	uint16_t idleChannel;						/*!< The ID of the channel permanently marked as idle (the 8th). */
};

#endif // TmMasterChannel_h
