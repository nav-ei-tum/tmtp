#ifndef TmPhysicalChannel_h
#define TmPhysicalChannel_h

#include "myErrors.h"
#include "TmFrameTimestamp.h"
#include "TmFrameBitrate.h"

#include <vector>
#include <string>
#include <stdint.h>

using namespace std;

class TmMasterChannel;	// Uses the TmMasterChannel class.
class TmFrameTimestamp;
class TmFrameBitrate;

/*! \brief Simulates/implements a physical channel.
 *
 * According to the ECSS-E-ST-50-03C Standard, a data channel carrying a stream of bits in a single direction is referred to as a physical channel. \n
 * The value of the Transfer Frame Version Number is constant for all frames of a physical channel. \n
 * The length of the frames for a given physical channel is fixed for a mission phase. \n
 *  
 * The physical channel can be divided into one or more master channels. \n
 * A master channel consists of all the frames with the same TF Version Number and Spacecraft ID. \n
 * Typically, there is only one master channel within a physical channel. However, multiple master channels can share a physical channel as well.
 * 
 * Each master channel can be divided into one or more virtual channels with a max of eight VCs per master channel.
 */
class TmPhysicalChannel {
//
// methods
//
	public:
	
/*! \brief Constructor of the TmPhysicalChannel class.
 *	\param length Sets the total frame length (7 to 2048 Bytes).
 *
 * Establishes the total frame length to have on this channel.
 * It sets the Frame Error Control Field Flag to FALSE and the Master Channel pointer to NULL.
 *
 * \note
 * If the total frame length falls out-of-bounds, a TmPhysicalChannelError will be thrown.
 */
		TmPhysicalChannel(uint16_t length);

/*! \brief Destructor of the TmPhysicalChannel class.
 *
 * Deletes the generated master channel.
 */
		~TmPhysicalChannel();

/*! \brief Retrieves the total frame length value. */
		virtual uint16_t getFrameLength();

/*! \brief Sets the FECF Flag to TRUE. */
		virtual void activateFecf();

/*! \brief Sets the FECF Flag to FALSE. */
		virtual void deactivateFecf();

/*! \brief Retrieves the value of the  FECF Flag. */
		virtual bool getFecfStatus();

/*! \brief Creates a master channel bound to this physical channel.
 *	\param scid The Spacecraft Identifier used in the master channel.
 *
 * The master channel will have the following attributes:
 * 	- Spacecraft ID = scid;
 * 	- ocfPresent = true;
 * 	- sendFrameCount = 0;
 * 	- recFrameCount = 0;
 * 	- secondHeaderPresent = false;
 * 	- extendedVcFrameCountUsed = false;
 * 	- ocfSink = NULL;
 * 	- currentVc = 0;
 * 	- idleChannel = 7;
 *
 * Links the master channel with this physical channel through the "this" pointer. \n
 * Creates a vector of eight virtual channels. All, except the last one, are empty (they point to NULL). \n
 * The last one is considered idle and it is created with the following (default) attributes:
 * 	- virtualChannelId = 7;
 * 	- packetSink = NULL;
 * 	- secondHeaderPresent = false;
 * 	- extendedFrameCountSet = false;
 * 	- dataFieldSynchronised = true;
 * 	- initialConf = netProtConf = new NetProtConf;
 * 	- debugOutput = false;
 * 	- sendFrameCount = 0;
 * 	- recFrameCount = 0;
 * 	- recPacketHeaderLength = 0;
 * 	- recPacketLength = 0;
 * 
 * \note A TmPhysicalChannelError is thrown if:
 * 	- The Spacecraft ID does not fall within the range of 0 to 1023.
 * 	- The idle Virtual Channel ID does not fall within the range of 0 to 7.
 */
		virtual TmMasterChannel* createTmMasterChannel(uint16_t scid);

/*! \brief Unwraps and analyzes a raw frame for master/virtual channel setting discrepancies and displays the corresponding warnings.
 *	\param rawFrame An encapsulated (raw) frame as it was received.
 *	\param timestamp A TmFrameTimestamp object. Contains the frame timestamp (as a reference) used to calculate individual packet timestamps.
 *	\param bitrate A TmFrameBitrate object. Contains the frame bitrate used to calculate individual packet timestamps.
 *	\return Any warnings or errors found in any step in the process.
 *
 * Creates a TM Transfer Frame object to receive the data carried by the raw frame.
 * Checks the FECF Flag settings for this physical channel and activates it in the new frame if needed.
 * Takes the raw frame, reads its fields and flags and stores them in the new frame.
 * If a master channel has been defined for this physical channel, verifies if the received frame has the correct master channel settings. 
 * Otherwise displays a warning that no master channel has been configured.
 * Scans for any TM Transfer Frame errors and returns its findings.
 */
		virtual TmChannelWarning receiveFrame(vector<uint8_t> rawFrame, TmFrameTimestamp timestamp, TmFrameBitrate bitrate);

/*! \brief Prepares a new TM Transfer Frame to be sent over a master- and virtual channel, if defined.
 * \param timestamp The timestamp at which the frame will be send.
 *
 * \note May throw TmPhysicalChannelError or TmMasterChannelError.
 */
		virtual vector<uint8_t> sendFrame(TmFrameTimestamp timestamp);

	// variables
	protected:
		TmMasterChannel *masterChannel;	/**< Pointer to the generated master channel. */
		uint16_t frameLength;		/**< Total frame length. */
		bool fecfPresent;				/**< Frame Error Control Field flag (default = FALSE). */
};

#endif // TmPhysicalChannel_h
