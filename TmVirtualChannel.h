#ifndef TmVirtualChannel_h
#define TmVirtualChannel_h

#include "NetProtConf.h"
#include "myErrors.h"
#include "TmFrameTimestamp.h"
#include "TmFrameBitrate.h"
#include "ticp/TicpTimestamp.hpp"

#include <boost/function.hpp>

#include <vector>
#include <queue>
#include <stdint.h>

using namespace std;

// Uses the following classes:
class TmTransferFrame;
class TmMasterChannel;
class GroundPacketServer;
class TmFrameTimestamp;
class TmFrameBitrate;
//class NetProtConf;

/*! \brief Offers a way to store a packet, its timestamp and the bitrate at reception time, together in a single structure.*/
struct TimeTaggedPacket {
	vector <uint8_t> data;		/*!< The packet - Consists of a vector of Bytes.*/
	TmFrameTimestamp timestamp;	/*!< The timestamp - Consists of seconds and fractions of seconds*/
	TmFrameBitrate bitrate;		/*!< The bitrate - Reference bitrate in bits per second upon which the timestamp was computed.*/
};

/*! \brief Implementation of a TMTP Virtual Channel.
 *
 * According to the ECSS-E-ST-50-03C standard virtual channels enable one physical channel to be shared among multiple higher-layer data streams. \n
 * There are a maximum of eight virtual channels in a given master channel.
 */
class TmVirtualChannel {
//
// definitions
//
protected:
	static const uint16_t recPacketBufferSize = 100;		/*!< Maximum amount of received enhanced packets (and their timestamps) waiting for processing. */
	static const uint16_t sendPacketBufferSize = 100;		/*!< Maximum amount of packets waiting to be sent. */

//
// methods
//
public:

/*! \brief Constructor of the TmVirtualChannel class.
 *	\param id The Virtual Channel ID for this virtual channel.
 *	\param *parent The master channel to which this virtual channel belongs.
 * 
 * Creates a virtual channel with the following (default) attributes:
 *		- virtualChannelId = id;
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
 * Links this virtual channel with a master channel through the *parent pointer. 
 * 
 * \note 
 * A TmVirtualChannelError is thrown if the idle Virtual Channel ID does not fall within the range of 0 to 7.
 * Also, the initial configuration assumes all contained packets are idle.
 */
	TmVirtualChannel(uint16_t id, TmMasterChannel *parent);
	
/*! \brief Destructor of the TmVirtualChannel class.
 *
 * Removes the generated initial config object from memory.
 */
    ~TmVirtualChannel();

/*! \brief Retrieves the Virtual Channel ID. */
	virtual uint16_t getVirtualChannelId();

/*! \brief Retrieves the sent frame counter. */
	virtual uint64_t getSendFrameCount();

/*! \brief Retrieves the received frame counter. */
	virtual uint64_t getRecFrameCount();

/*! \brief Sets the Secondary Header and the extended frame counter flags to TRUE. */
	virtual void activateExtendedFrameCount();

/*! \brief Sets the Secondary Header and the extended frame counter flags to FALSE. */
	virtual void deactivateExtendedFrameCount();
	
/*! \brief Retrieves the value of the extended frame counter flag. */
	virtual bool getExtendedFrameCountStatus();

/*! \brief Sets the direct Data Field access flag to TRUE. */
	virtual void activateDirectDataFieldAccess();
	
/*! \brief Sets the direct Data Field access flag to FALSE. */
	virtual void deactivateDirectDataFieldAccess();
	
/*! \brief Retrieves the value of the direct Data Field access flag. */
	virtual bool getDirectDataFieldAccessStatus();

/*! \brief Places a packet in the output queue.
 * \param packet A packet (vector of Bytes) ready to be queued and sent.
 *
 * If the output queue has not reached its limit, places the packet in the last position of the output queue.
 * Only if this packet is the very first in the queue, the sendPointer varaible will be set to point to its first vector element.
 *
 * \note 
 * If the output queue has reached its limit, will throw a TmVirtualChannelError.
 */
    virtual void sendPacket(vector<uint8_t> packet);
	
public:
/*! \brief Retrieves a packet with its timestamp from the input queue.
 *
 * If the input packet queue has an available packet, a TimeTaggedPacket structure is retrieved and popped out of the FIFO queue.
 * This member function returns said structure. The packet is contained within the structure, along with its timestamp.
 * It is the job of the GroundPacketServer class to separate both pieces of information.
 *
 * \note 
 * If the input queue is empty, will throw a TmVirtualChannelError.
 */
	virtual TimeTaggedPacket receivePacket();

/*! \brief Indicates whether there are any frames available to send.
 *
 * First it is verified if the Direct Data Field Access flag is TRUE. If it is, then we most likely have frames to send (for navigation data). \n
 * Otherwise, it checks if the output queue is not empty (i.e. there are frames waiting to be transmitted).
 */
	virtual bool frameAvailable();
	
/*! \brief Indicates whether the input packet queue has an available packet. */
	virtual bool packetAvailable();

/*! \brief Reads the Data Field of a received frame and extracts the packets it contains (must be unwrapped first).
 * \param frame The received frame to extract packets from.
 *
 * \note The usage of this member function assumes the frame was already unwrapped either at the physical- or master channel level.
 *
 * Retrieves the Data Field from the received frame, extracts its data and initializes recPointer to its 1st position. \n
 * Checks frame settings consistency by comparing the following values of the received frame against the VC settings:
 *	- Virtual Channel ID.
 *	- Secondary Header Flag.
 *	- Synchronization Flag.
 *
 * After the consistency check, the extended VC frame counter is extracted (if activated) and the frame contents are displayed (if debug output was enabled).\n
 * The local Rx Frame Counter is compared to the counter extracted from the frame - they should be equal. If not, then the pre-buffer for received packets is cleared. \n
 *
 * The reasoning behind this is that if a packet spans several frames, each chunk is stored in the pre-buffer. Only if all parts are received in sequence, 
 * the whole packet is transferred from the pre-buffer into the input queue. Otherwise, the whole packet is discarded (the incomplete pieces, that is) and both
 * the received packet length and received packet header length variables are set to zero. \n
 * In this case, the Rx Frame Counter is rectified, the counter for lost frames is updated and later displayed as a warning. \n
 *
 * After all the check-ups and counter updates, the data (if not idle) is extracted. \n
 * First thing is attempting to use the Direct Data Field Access method if configured/defined.
 * \note For more details on what the Direct Data Field Access is, check TmVirtualChannel::connectDirectRecvDataFieldAccessFunction.
 *
 * If not possible, then the "normal packet mode" for data extraction is used:
 *	- If the First Header Pointer indicates (FHP) indicates the received packet extends across multiple frames, the local FHP will point to the end of the Data Field. 
 Otherwise, it will point to the beginning of the 1st packet in the Data Field. 
 *	- The local FHP is used to read the received Data Field. Packet extraction is done at the FHP and iterated until it reaches the end of the Data Field.
 *	- If the pre-buffer is empty, then we are probably dealing with a new packet. \n
 *
 *	- If dealing with a new packet, the received packet pointer (initialized to point at the start of the Data Field) is re-synched to where the received FHP points, if needed.
 A re-synch means that a packet does not start at the beginning of the Data field. Perhaps a previous packet did not fit in the last frame sent, so the remaining chunk was put in the current frame, 
 right at the start of the Data Field. This chunk is just ignored if the pre-buffer is empty (previous packet was probably dropped for being incomplete). A re-synch will throw a warning.
 *	- If no re-synch is needed, the packet header is read to find out if we are dealing with an idle packet.
 *	- If the packet is idle, it will be ignored. Otherwise the packet length is extracted from its header and the packet (or chunk) is placed in the pre-buffer. \n
 *
 *	- If NOT dealing with a new packet, the packet length (received in a previous frame) is compared to the size of the pre-buffer to see if we reached the end of it.
 *	- If the not yet reached the end of the packet, the current chunk is placed in the pre-buffer. 
 *		- But if by adding this chunk we reached the end, the packet length is read from its header and stored in the local variable.
 *	- After successfully reading a complete packet, it is transferred from the pre-buffer to the input queue. The local variables for packet length and header length are cleared as well as the pre-buffer.
 *	- Finally, a new packet is signaled and accumulated in the warning messages. 
 *
 * Any errors occured in this process are returned within an instance of TmChannelWarning.
 *
 */
	virtual TmChannelWarning receiveFrame(TmTransferFrame frame);
	
/*! \brief Creates a new TM frame, adjusts its settings and populates its Data Field.
 * \param timestamp TICP timestamp (only used if transmitting navigation messages).
 *
 * This member function starts by creating a TM Transfer Frame and adjusting its settings to match those set in the master- and virtual channel it belongs. \n
 *	- Default First Header Pointer = fhpNoFirstHeader.
 *
 * Settings taken from the master channel:
 *	- Frame length.
 *	- OCF Flag.
 *	- FECF flag.
 *
 * Settings taken from the virtual channel:
 *	- Virtual Channel ID.
 *	- Secondary Header Flag.
 *	- Extended frame counter flag.
 *	- Synchronization Flag.
 * 
 * After the frame has been adjusted, the data is prepared to be encapsulated. For this purpose it uses a vector of bytes to host the packets to be sent. \n
 * If using the Direct Data Field Access (DDFA) method to insert "raw data" in the Data Field:  \n
 * Checks whether a transmission DDFA wrapper function has been defined and uses it to process the raw data and store it in the data vector.
 * 
 * If using the "normal packet mode" to insert packets in the Data Field: \n
 *	- Computes the amount of the available Data Field space to store data.
 *	- Calculates the length of the 1st (not yet sent) packet in the output queue (if not empty).
 * 					
 * If the packet does NOT fit in the Data Field: \n
 *	- Checks whether the packet is the very first in the queue and that the FHP is set to the default. In which case the FHP shall point to the beginning of the data.
 *	- It then inserts as many Bytes from the output queue into the end of the data vector as they can fit in the Data Field and updates the sendPointer. \n
 *	- In the case of oversized packets the sendPointer will help us to place these packets in the data vector which could not fit in a single frame. \n
 *	- For instance, if only the half of a packet could be placed in the data vector, the sendPointer will show the location in the output queue of the last Byte that could be sent. \n
 *	- This way, next time a data vector is filled, it will begin at the position of sendPointer, thus inserting the rest of the packet. \n
 *	- More importantly, if this is the case, the FHP will not be updated! It will only point to the location where a new packet BEGINS. \n
 * 
 * If the packet fits in the Data Field: \n
 *	- Activates the debug messages and checks whether the packet is the very first in the queue and that the FHP is set to the default. In which case the FHP shall point to the beginning of the data.
 *	- Inserts the whole contents of one element of the output queue into the data vector (i.e. one whole packet).
 *	- Then, deletes current packet in the queue as it will be completely sent.
 *	- And updates the output-queue transmitted-bytes-pointer accordingly if the queue is empty.
 * 
 * This whole process will be repeated as long as the data vector still fits in the Data Field.
 * 
 * If there are no more packets in the output queue, but there is room left in the Data Field: \n
 *	- Checks whether a chunk of a big packet remained in the data vector, places the FHP to the end of the data and fills the rest of the data vector with idle packets. \n
 *	- However, if there was no data at all to send, sets the FHP to the predefined pattern to indicate idle contents and fills the rest of the data vector with idle packets. \n
 * 
 * After the data vector has been populated: \n
 *	- Sets the frame FHP as indicated by the previous process and inserts the data vector into the Data Field.
 * 
 * Finally, it updates the Sent Frame Counter and catches any errors.
 * 
 * As a result, a new frame born under the current virtual channel is returned.
 * 
 *	\note May throw TmVirtualChannelError.
 */
	virtual TmTransferFrame sendFrame(ticp::Timestamp timestamp);

/*!	\brief Establishes the sink where received packets will be placed (GroundPacketServer instance).
 *	\param sink Pointer to the packet sink to which this channel has been connected.
 */
	virtual void connectPacketSink(GroundPacketServer *sink);
	
/*!	\brief Sets the packet sink pointer to NULL. */
	virtual void disconnectPacketSink();

/*! \brief Establishes the packet configuration for this channel.
 *	\param conf Pointer to the Network Protocol Configuration instance.
 */
	virtual void setNetProtConf(NetProtConf *conf);

/*! \brief Specifies the function used to access the raw data in the Data Field in order to transmit.
 *	\param function A function object wrapper using the boost libraries.
 *
 * Receives a function wrapper called "function" (i. e. a placeholder for functions) which takes an unsigned short and a TICP timestamp and returns a vector of Bytes. \n
 * These Direct Data Field Access functions are used to implement alternative methods of extracting data.
 * The Data field may contain "raw data" with a very specific format and it could only be read in a very specific way. \n
 * For example, the contents may be navigation data, which is NOT encapsulated in packets. Therefore, writing the Data Field in "normal packet mode" will not work.
 */
	virtual void connectDirectSendDataFieldAccessFunction(
			boost::function<vector<uint8_t>(uint16_t, ticp::Timestamp)> function);

/*!	\brief Specifies the function used to access the raw data in the the Data Field of a received frame.
 *	\param function A function object wrapper using the boost libraries.
 *
 * Receives a function wrapper called "function" - In other words, it is a placeholder for functions. 
 * In this case it takes an vector of bytes (passed by reference) and a TICP timestamp and returns void. \n
 * These Direct Data Field Access functions are used to implement alternative methods of extracting data.
 * The Data field may contain "raw data" with a very specific format and it could only be read in a very specific way. \n
 * For example, the contents may be navigation data, which is NOT encapsulated in packets. Therefore, reading the Data Field in "normal packet mode" will not work.
 */
	virtual void connectDirectRecvDataFieldAccessFunction(
			boost::function<void(vector<uint8_t> const&, ticp::Timestamp)> function);

/*! \brief Sets the debug output flag to TRUE. */
	virtual void activateDebugOutput();

/*! \brief Sets the debug output flag to FALSE. */
	virtual void deactivateDebugOutput();

protected:

/*! \brief  Retrieves the 1st packet in the packet sink reception queue (if any) and displays the corresponding debug messages.
 *
 * Creates and returns a TmChannelWarning instance with any warnings or error messages generated.
 * If a packet sink is defined (instance of GroundPacketServer), retrieves the 1st message in the input queue of the packet sink 
 * and optionally displays debug messages.
 */
	virtual TmChannelWarning signalNewPacket();

//
// variables
//
protected:
	TmMasterChannel *masterChannel;		/*!< Pointer to the master channel this VC belongs to. */
	GroundPacketServer *packetSink;		/*!< GroundPacketServer instance receiving packets. */
	NetProtConf *netProtConf;			/*!< Actual packet configuration - Can be defined with TmVirtualChannel::setNetProtConf , but not defined here. Perhaps in the main? */
	NetProtConf *initialConf;			/*!< Initial packet configuration - Defined nowhere. Uses default NetProtConf settings (all idle). */

	// attributes set for this virtual channel
    uint16_t virtualChannelId;	/*!< Virtual Channel ID. */
    uint64_t sendFrameCount;	/*!< Transmitted frames counter. */
    uint64_t recFrameCount;		/*!< Received frames counter. */
    bool secondHeaderPresent;	/*!< Secondary Header Flag. */
	bool extendedFrameCountSet;	/*!< (Not part of the standard) Locally used flag to indicate usage of the Extended VC Frame Counter.*/
    bool dataFieldSynchronised;	/*!< The Synchronization Flag indicates the formatting of the Transfer Frame Data Field.
											CAUTION! This variable is misleading: if dataFieldSynchronised = TRUE, it means the Synchronization Flag = FALSE*/
	bool debugOutput;			/*!< Indicates whether any debug messages are to be displayed or not (default = FALSE). */
	bool directDataFieldAccess;	/*!< Indicates the Data Field can be accessed directly (by means of a function wrapper). */
	
	// Function object wrappers - boost library.
	/*! Placeholder for a function that implements direct Data Field access to prepare packets for transmission. */
	boost::function<vector<uint8_t>(uint16_t, ticp::Timestamp)> directSendDataFieldAccessFunction;
	
	/*! Placeholder for a function that implements direct Data Field access to read received packets. */
	boost::function<void(vector<uint8_t> const&, ticp::Timestamp)> directRecvDataFieldAccessFunction;

	// buffers
	queue<vector<uint8_t> > sendFifo;			/*!< Output queue - Temporarily stores packets before sending them. */
	queue<TimeTaggedPacket> recFifo;				/*!< Input queue - Stores received packets and their respective Timestamp (together as TimeTaggedPacket structs) 
														before sending them to the ground packet server. */
	vector<uint8_t>::iterator sendPointer;		/*!< Position last Byte in the output queue which was sucessfully encapsulated in a frame. 
														Useful to encapsulate chunks of oversized packets across several frames. */
	vector<uint8_t> recPacket;					/*!< Pre-Buffer for received packets. Stores pieces of packets that span several frames, before putting the entire packet in the queue. */
	uint64_t recPacketHeaderLength;				/*!< Packet header length according to the NetProtConf settings. */
	uint64_t recPacketLength;					/*!< Total Packet length stored in the Packet Header and extracted as specified in NetProtConf. */
};

#endif // TmVirtualChannel_h
