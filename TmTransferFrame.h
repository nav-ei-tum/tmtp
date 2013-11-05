#ifndef TmTransferFrame_h
#define TmTransferFrame_h

#include "TmOcf.h"
#include "myErrors.h"
#include "TmFrameTimestamp.h"
#include "TmFrameBitrate.h"

#include <vector>
#include <stdint.h>

using namespace std;

/*! \brief Implementation of the Telemetry Transfer Frame for the Telemetry Transfer Protocol (TMTP).
 * 
 * As specified in the ECSS standard ECSS-E-ST-50-03C:
 * The TM Transfer Frame shall encompass the major fields, positioned contiguously if present, in the sequence shown:
 * 
 * \image html TM_Frame_Overview_ver2.png "The Telemetry Transfer Frame format at a glance." 
 *
 */

class TmTransferFrame {
//
// definitions
//
public:
	static const uint16_t fhpNoFirstHeader = 0x07FF;	/**< Predefined contents of the First Header Pointer if no packet starts in the TF Data Field.
															This is used if a long packet spans across multiple frames.*/
																
	static const uint16_t fhpOnlyIdleData = 0x07FE;	/**< Predefined contents of the First Header Pointer if the TF Data Field containts idle data.*/

protected:
	static const uint16_t transferFrameVersion = 0;	/**< The TF Ver. Number shall be '00' for all TM Transfer Frames (as defined in CCSDS 135.0-B-3).*/
	static const uint16_t secondHeaderVersion = 0;	/**< SH Ver. Number '00' (Version 1), is the only one recognized by the ECSS-E-ST-50-03C standard.*/
	static const uint16_t primaryHeaderLength = 6;	/**< 6-Byte Primary Header.*/
	static const uint16_t fecfLength = 2;				/**< Frame Error Control Field (optional) is 2 Bytes long.*/

//
// methods
//
public:

/*! \brief Constructor for the TmTransferFrame class.
 *  \param length The length of the whole TM Transfer Frame (7 to 2048 Bytes).
 *
 * Creates a Telemetry Transfer Frame with the following (default) attributes:
 *	- spacecraftId = 0;
 *	- virtualChannelId = 0;
 *	- ocfPresent = false;	
 *	- masterChannelFrameCount = 0;
 *	- virtualChannelFrameCount = 0;
 *	- secondHeaderPresent = false;
 *	- extendedVcFrameCount = false;
 *	- dataFieldSynchronised = false;
 *	- firstHeaderPointer = 0;
 *	- fecfPresent = false;
 *
 * \note
 * A TmTransferFrameError is thrown if the length does not fall within the range of 7 to 2048 Bytes.
 */
    TmTransferFrame(uint16_t length);

/*! \brief Retrieves the Transfer Frame Version Number. */
	virtual uint16_t getTransferFrameVersion();

/*! \brief Sets the Spacecraft Identifier.
 *  \param id The Spacecraft Identifier (0-1023).
 *
 * \note
 * The Spacecraft ID is 10 bits long. If "id" does not fall within the range of 0 to 1023, will throw TmTransferFrameError.
 */
    virtual void setSpacecraftId(uint16_t id);
	
/*! \brief Retrieves the Spacecraft Identifier. */
	virtual uint16_t getSpacecraftId();

/*! \brief Sets the Virtual Channel Identifier.
 *  \param id The Virtual Channel (0-7).
 *
 * \note
 * The Virtual Channel ID is 3 bits long. If "id" does not fall within the range of 0 to 7, will throw TmTransferFrameError.
 */
    virtual void setVirtualChannelId(uint16_t id);

/*! \brief Retrieves the Virtual Channel Identifier. */
	virtual uint16_t getVirtualChannelId();

/*! \brief Sets the Operational Control Field Flag to TRUE. */
	virtual void activateOcf();

/*! \brief Retrieves the value of the Operational Control Field Flag. */
	virtual bool getOcfStatus();

/*! \brief Inserts a 4-Byte long Operational Control Field object.
 *  \param ocfObject The 4-Byte long OCF to be insterted in the TF Trailer.
 */
	virtual void setOcf(TmOcf ocfObject);

/*! \brief Retrieves the Operational Control Field currently in the TF Trailer. */
	virtual TmOcf getOcf();

/*! \brief Sets the Frame Error Control Field flag to TRUE. */
	virtual void activateFecf();

/*! \brief Retrieves the value of the Frame Error Control Field flag. */
	virtual bool getFecfStatus();

/*! \brief Sets the Master Channel Frame Counter.
 *  \param count The new value of the Counter.
 *
 * \note
 * The Master Channel Frame Counter is 8 bits long. If "count" does not fall within the range of 0 to 255, will throw TmTransferFrameError.
 */
    virtual void setMasterChannelFrameCount(uint16_t count);

/*! \brief Retrieves the Master Channel Frame Counter. */
	virtual uint16_t getMasterChannelFrameCount();

/*! \brief Sets the Virtual Channel Frame Counter.
 *  \param count The new value of the Counter.
 *
 * \note
 * The Virtual Channel Frame Counter is 8 bits long. If "count" does not fall within the range of 0 to 255, will throw TmTransferFrameError.
 */
    virtual void setVirtualChannelFrameCount(uint64_t count);

/*! \brief Retrieves the Virtual Channel Frame Counter. */
	virtual uint64_t getVirtualChannelFrameCount();

/*! \brief Sets the Secondary Header flag to TRUE. */
	virtual void activateSecondHeader();

/*! \brief Retrieves the value of the Secondary Header flag. */
	virtual bool getSecondHeaderStatus();

/*! \brief Activates the extended VC Frame Counter and fixes the SH Data Field (if empty) to three Bytes filled with zeroes or (if not empty and 3-Bytes long) copies its contents into the Frame Counter.
 *  
 * If the Secondary Header Data Field is present and has already some data, this function checks if it is 3 Bytes long. \n
 * A length of 3 Bytes could mean that the contents are an extended counter. In this the case, it DOES NOT FORCE the
 * Extended VC Frame Counter activation. Instead, it leaves whatever is stored there intact and copies its contents 
 * into the last three Bytes of the VC Frame Counter. Otherwise, an error is thrown.
 *
 * \note May throw TmVirtualChannelError. 
 */
	virtual void activateExtendedVcFrameCount();

/*! \brief Retrieves the value of the Extended VC Frame Conter flag. */
	virtual bool getExtendedVcFrameCountStatus();

/*! \brief Retrieves the value of the Secondary Header Version. */
	virtual uint16_t getSecondHeaderVersion();

/*! \brief Retrieves the length of the Secondary Header Data Field. */
	virtual uint16_t getSecondHeaderLength();

/*! \brief Populates the Secondary Header Data Field.
 *  \param data The data to store in the Secondary Header Data Field.
 *
 * \note May throw TmTransferFrameError. 
 * If the Extended Frame Counter is not being used and "data" and it is less than or equal to the max. SH Data Field Length, then the data is stored.
 * Otherwise an error is thrown.
 */
	virtual void setSecondHeaderDataField(vector<uint8_t> data);

/*! \brief Retrieves the contents of the Secondary Header Data Field. */
	virtual vector<uint8_t> getSecondHeaderDataField();

/*! \brief Sets the Synchronization Flag to TRUE. */
    virtual void activateDataFieldSynchronisation();

/*! \brief Retrieves the value of the Synchronization Flag. */
	virtual bool getDataFieldSynchronisationStatus();

/*! \brief Sets the location in the TM Data Field where the 1st Byte of a new packet starts.
 *  \param location The position (i.e. in which Byte) of the TM Data Field a new packet begins.
 *
 * It is assumed that the Synchronization Flag is '0', because otherwise, the FHP would be "undefined". \n
 * If at least one packet begins in the TM Data Field, the FHP will show in which Byte of the field can 
 * the 1st Byte of the packet be found. \n
 * The locations of the Bytes in the TM Data Field are numbered in ascending order starting with zero. \n
 * If a packet spans across multiple frames, the FHP will contain the pattern 111 1111 1111 to indicate 
 * that no new packet begins in the TM Data Field. (see definition for fhpNoFirstHeader) \n
 * In order to indicate that the TM Data Field contains idle data, the pattern 111 1111 111o will be used.
 * (see definition for fhpOnlyIdleData)
 *
 * \note May throw TmTransferFrameError.
 */
    virtual void setFirstHeaderPointer(uint16_t location);

/*! \brief Retrieves the value of the First Header Pointer. */
	virtual uint16_t getFirstHeaderPointer();

/*! \brief Retrieves the length of the whole TM Transfer Frame. */
    virtual uint16_t getLength();

/*! \brief Calculates the TM Data Field length.
 *  
 * It subtracts the Primary Header, Secondary Header, OCF and FECF lengths from the total TM Transfer Frame length.
 * The returned value can also be zero.
 */
    virtual uint16_t getDataFieldLength();

/*! \brief Calculates the Secondary Header length.
 *
 * It subtracts the Primary Header, minimum TM Data Field (1), OCF and FECF lengths from the total TM Transfer Frame length.
 * The result is truncated if greater than 64 (standard specified maximum). 
 */
	virtual uint16_t getMaxSecondHeaderLength();

/*! \brief Populates the TM Data Field.
 *  \param data The contents of the TM Data Field, which must match exactly in size.
 *
 * \note
 * If "data" has not exactly the same length as the TM Data Field, a TmTransferFrameError is thrown.
 */
    virtual void setDataField(vector<uint8_t> data);

/*! \brief Retrieves the data stored in the TM Data Field as obtained from the unwrap() function.
 * 
 * It could simply return variable dataField, but it is protected. It returns a variable called retVec instead. \n
 * This variable receives a copy of the contents of dataField. Any difference in length will be either filled with zeroes or truncated.
 * The dataField has to be obtained with the unwrap() function first!
 */
	virtual vector<uint8_t> getDataField();

/*! \brief Builds the different frame fields and encapsulates a packet in it.
 * 
 * It requires an already created and populated Data Field (use setDataField(vector<uint8_t>) \n
 * Bare in mind that this function calls the tmOcf::wrap() function, which in order to work properly for SENDING, the following functions must be run in sequence:
 *	1.- setReportType(ReportType type)
 *	2.- setContent(uint64_t data)
 *	3.- wrap()
 * 
 * The first step in the wrapping process is to verify the Data Field Length is not zero. Therefore, the optional fields must be activated and populated as needed. \n
 * Before calling the wrap function, it is required to activate the optional fields as needed in sequence:
 *
 * 1.- The Secondary Header: 
 *	- TmTransferFrame::activateSecondHeader()
 *
 * 2.- Extended Virtual Circuit Frame Counter:
 *	- TmTransferFrame::activateExtendedVcFrameCount()
 *
 * 3.- The Operational Control Field:
 *	- TmOcf::setReportType(ReportType type)
 *	- TmOcf::setContent(uint64_t data)
 *	- TmOcf::wrap()
 *	- TmTransferFrame::activateOcf()
 *
 * 4.- The Frame Error Control Field:
 *	- TmTransferFrame::activateFecf()
 *
 * \note
 * If the Data Field length is zero, a TmTransferFrameError will be thrown.
 *
 * \warning The usage of dataFieldSynchronised can be misleading: 
 * Here it is implied that when dataFieldSynchronised = TRUE, it means the Synchronization Flag = FALSE!
 * Refer to ECSS-E-ST-50-03C, clause 5.2.7.3
 */
    virtual vector<uint8_t> wrap();

/*! \brief Takes a TMTP Frame, reads the Fields and Flags and stores their values in local variables accordingly.
 *  \param raw The TMTP Frame generated by TmTransferFrame::wrap().
 *
 * \note may throw TmTransferFrameError.
 */
	virtual void unwrap(vector<uint8_t> raw);

/*! \brief Dissects the frame into its components and displays them as messages for debugging.
 *
 * Displays the following information:
 *	- Transfer frame length (Bytes).
 *	- Virtual Channel ID.
 *	- Operational Control Field Flag.
 *	- Master Channel Frame Counter.
 *	- Virtual Channel Frame Counter.
 *	- Secondary Header Flag.
 *	- First Header Pointer
 *	- Whether the Frame Error Control Field is in use.
 *	- The contents of the TM Data Field.
 *	- The TM Data Field length (Bytes).
 *
 * Does not display the following fields:
 *	- Master Channel ID.
 *	- Sinchronization Flag.
 *	- Packet Order Flag.
 *	- Segment Length ID.
 *	- Secondary Header.
 *	- Operational Control Field contents.
 */
	virtual void debugOutput();

/*! \brief Inserts a timestamp object.
 *  \param timestampObject Reference timestamp used to compute each individual packet's timestamp.
 */
	virtual void setTimestamp(TmFrameTimestamp timestampObject);

/*! \brief Retrieves the stored TmFrameTimestamp object. */
	virtual TmFrameTimestamp getTimestamp();

/*! \brief Inserts a bitrate object.
 *  \param bitrateObject Reference bitrate used to compute each individual packet's timestamp.
 */
	virtual void setBitrate(TmFrameBitrate bitrateObject);

/*! \brief Retrieves the stored TmFrameBitrate object. */
	virtual TmFrameBitrate getBitrate();	
	
protected:

/*! \brief Calculates the position (in Bytes) where the TM Data Field starts.
 * 
 * start = primaryheaderLength + secondHeaderLength
 */
	virtual uint16_t getDataFieldStart();

/*! \brief Calculates the position (in Bytes) where the TM Data Field ends.
 *
 * end = frameLength - (ocfLength + fecfLength)
 */
	virtual uint16_t getDataFieldEnd();

/*! \brief Computes a Cyclic Redundancy Check on a message (frame). 
 *  \param message The whole TM Transfer Frame (if sending, the Frame Error Control Field should be excluded).
 *
 * As per the ECSS-E-ST-50-03C standard, the ENCODING PROCEDURE is as follows: \n
 * 1. The encoding procedure accepts an (n-16)-bit TM Transfer Frame, excluding the Frame Error Control Field, 
 * and generates a systematic binary (n,n-16) block code by appending a 16-bit Frame Error Control Field as 
 * the final 16 bits of the codeblock.
 *
 * 2. The equation for the contents of the Frame Error Control Field is:
 * FECF = [(X^16 * M(X)) + (X^(n-16) * L(X))] modulo G(X)
 * 
 * where:
 * - All arithmetic is modulo 2.
 * - FECF is the 16-bit Frame Error Control Field and the first bit transferred is the most significant bit, 
 * taken as the coefficient of the highest power of X.
 * - n is the number of bits in the encoded message;
 * - M(X) is the (n-16)-bit information message to be encoded expressed as a polynomial with binary coefficients, 
 * and the first bit transferred is the most significant bit, M0, taken as the coefficient of the highest power of X.
 * - L(X) is the presetting polynomial given by:
 *		L(X) = Sumation from i=0 to 15 of X^i
 * - G(X) is the generating polynomial given by: \n
 *		G(X) = X^16 + X^12 + X^5 + 1
 * 
 * \note 
 * The X^(n-16) * L(X) term has the effect of presetting the shift register to all '1' state prior to encoding.
 * 
 * The DECODING PROCEDURE uses an error detection syndrome, S(X), given by: \n
 * S(X) = [(X^16 * C'(X)) + (X^n * L(X))] modulo G(X)
 *
 * where:
 * - C'(X) is the received block, including the Frame Error Control Field, in polynomial form, 
 * with the first bit transferred being the most significant bit C0 taken as the coefficient of the highest power of X.
 * - S(X) is the syndrome polynomial which is zero if no error is detected and non-zero if an error is detected, 
 * with the most significant bit S0 taken as the coefficient of the highest power of X.
 */
	virtual uint16_t crc(vector<uint8_t> message);

//
// variables
//
protected:
    uint16_t spacecraftId;					/*!< Spacecraft ID is 10 bits long.*/
    uint16_t virtualChannelId;				/*!< Virtual Chanel ID is 3 bits long.*/
    bool ocfPresent;						/*!< Operational Control Field Flag.*/
    uint16_t masterChannelFrameCount;		/*!< Master Channel Frame Counter, should be 1 Byte long (modulo 256 counter).*/
    uint64_t virtualChannelFrameCount;		/*!< Virtual Channel Frame Counter, 1 Byte long. Can be extended up to 4 Bytes.*/
    bool secondHeaderPresent;				/*!< Secondary Header Flag.*/
	bool extendedVcFrameCount;				/*!< (Not part of the standard) Locally used flag to indicate usage of the Extended VC Frame Counter.*/
	vector<uint8_t> secondHeaderDataField;	/*!< (Optional) If Secondary Header present, its Data Field can be up to 63 Bytes long.*/
    bool dataFieldSynchronised;				/*!< The Synchronization Flag indicates the formatting of the Transfer Frame Data Field.
														CAUTION! This variable can be misleading: if dataFieldSynchronised = TRUE, it means the Synchronization Flag = FALSE*/
    uint16_t firstHeaderPointer;			/*!< The First Header Pointer contains the location of the 1st Byte of the 1st packet starting in the TM Data Field. This is 11 bits long.*/
    vector<uint8_t> dataField;				/*!< Contents of the TM Data Field. The TM Data Field has a variable length and a min. of 1.*/ 
	TmOcf ocf;								/*!< (Optional) Operational Control Frame Field. This local variable stores an entire TmOcf object. This is NOT directly inserted in the Frame.*/
	bool fecfPresent;						/*!< (Not part of the standard) Locally used flag to indicate usage of the Frame Error Control Field.*/
    uint16_t frameLength;					/*!< (Not part of the standard) Locally used variable to store the total frame length in Bytes. 
														Required parameter when instantiating this class. Must be 7 to 2048 Bytes long.*/
	TmFrameTimestamp referenceTimestamp;	/*!< (Optional and not part of the standard) Locally used timestamp to serve as reference to compute the timestamp of each packet.*/
	TmFrameBitrate referenceBitrate;		/*!< (Optional and not part of the standard) Locally used bitrate to compute the timestamp of each packet*/
};

#endif // TmTransferFrame_h
