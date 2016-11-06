#ifndef __DPA_MESSAGE
#define __DPA_MESSAGE

#include "DpaWrapper.h"
#include <memory>
#include <string>
#include <cstdint>

///< Size of buffer for message.
#define MAX_DPA_BUFFER    64

class DpaMessage {
 public:
  /** Size of the maximum DPA message. */
  static const int kMaxDpaMessageSize = MAX_DPA_BUFFER;
  /** Address for broadcast messages */
  static const uint16_t kBroadCastAddress = BROADCAST_ADDRESS;
  /** Defines an alias representing the union. */
  typedef union {
	uint8_t Buffer[MAX_DPA_BUFFER];
	struct {
	  uint16_t NADR;
	  uint8_t PNUM;
	  uint8_t PCMD;
	  uint16_t HWPID;
	  uint8_t ResponseCode;
	  uint8_t DpaValue;
	  TDpaMessage DpaMessage;
	} DpaResponsePacket_t;
	struct {
	  uint16_t NADR;
	  uint8_t PNUM;
	  uint8_t PCMD;
	  uint16_t HWPID;
	  TDpaMessage DpaMessage;
	} DpaRequestPacket_t;
  } DpaPacket_t;

  /** Values that represent message types. */
  enum MessageType {
	///< Request message
	kRequest,
    ///< Confirmation message
	kConfirmation,
    ///< Notification message
	kNotification,
    ///< Response message
	kResponse
  };

  /** Default constructor. */
  DpaMessage();

  /** Constructor from data */
  DpaMessage(const unsigned char* data, uint32_t length);

  /** Constructor from string */
  DpaMessage(const std::basic_string<unsigned char>& message);

  /**
   Copy constructor.
  
   @param	other	The original message.
   */
  DpaMessage(const DpaMessage& other);

  /** Destructor. */
  virtual ~DpaMessage();

  /**
   Assignment operator.
  
   @param	other	The original message.
  
   @return	A shallow copy of this object.
   */
  DpaMessage& operator=(const DpaMessage& other);

  /**
  Assignment operator.

  @param	message string to be assigned to buffer

  @return	this with assigned data
  */
  DpaMessage& operator=(const std::basic_string<unsigned char>& message);

  /**
   Gets message type.
  
   @return	A MessageType.
   */
  MessageType MessageDirection() const;

  /**
   Fills message with data from IQRF network.

   @exception   std::invalid_argument   Thrown when data length is 0.
   @exception	std::invalid_argument	Thrown when data is nullptr.
   @exception	std::length_error	 	Raised when a length is bigger than max buffer size.

   @param [in,out]	data	Pointer to data.
   @param	length			The number of bytes to be added.
   */
  void FillFromResponse(const unsigned char* data, uint32_t length);

  /**
   Adds data to message buffer.

   @exception   std::invalid_argument   Thrown when data length is 0.
   @exception	std::invalid_argument	Thrown when data is nullptr.
   @exception	std::length_error	 	Raised when a length is bigger than max buffer size.

   @param [in,out]	data	Pointer to data.
   @param	length			The number of bytes to be added.
   */
  //TODO error here?
  void AddDataToBuffer(const unsigned char* data, uint32_t length);

  /**
  Stores data to message buffer.

  @exception   std::invalid_argument   Thrown when data length is 0.
  @exception	std::invalid_argument	Thrown when data is nullptr.
  @exception	std::length_error	 	Raised when a length is bigger than max buffer size.

  @param [in,out]	data	Pointer to data.
  @param	length			The number of bytes to be stored.
  */
  void DataToBuffer(const unsigned char* data, uint32_t length);

  /**
   Gets length of data stored in message.
  
   @return	Number of bytes in message.
   */
  int Length() const { return length_; }

  /**
   Gets destination or source address of sender/receiver.
  
   @return	An address.
   */
  uint16_t NodeAddress() const { return dpa_packet_->DpaRequestPacket_t.NADR; }

    /**
   Gets peripheral type.
  
   @return	A peripheral type.
   */
  TDpaPeripheralType PeripheralType() const { return TDpaPeripheralType(dpa_packet_->DpaRequestPacket_t.PNUM); }

  /**
   Gets command code.
  
   @return	A command code.
   */
  uint8_t CommandCode() const { return dpa_packet_->DpaResponsePacket_t.PCMD; }

  /**
   Gets response code from received message.

   @exception	unexpected_packet_type	Thrown when message is not a receive type.

   @return	A response code.
   */
  TErrorCodes ResponseCode() const;

  /**
   Gets DPA packet behind the message.
  
   @return	A reference to a const DpaPacket_t.
   */
  const DpaPacket_t& DpaPacket() const { return *dpa_packet_; }

  /**
   Gets pointer to data stored in message.
  
   @return	Pointer to data stored in message.
   */
  unsigned char* DpaPacketData() { return dpa_packet_->Buffer; }
  const unsigned char* DpaPacketData() const { return dpa_packet_->Buffer; }

 private:
  DpaPacket_t* dpa_packet_;
  const int kCommandIndex = 0x03;
  const int kStatusCodeIndex = 0x06;
  int length_;

  bool IsConfirmationMessage() const;
};

#endif // !__DPA_MESSAGE

