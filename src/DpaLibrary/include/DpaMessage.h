#ifndef __DPA_MESSAGE
#define __DPA_MESSAGE

#include "DpaWrapper.h"
#include <memory>
#include <cstdint>


#define MAX_DPA_BUFFER    64

class DpaMessage {
 public:
  const int kMaxDpaMessageSize = MAX_DPA_BUFFER;
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

  enum MessageType {
	kRequest,
	kConfirmation,
	kNotification,
	kResponse
  };

  DpaMessage();
  DpaMessage(const DpaMessage& other);
  virtual ~DpaMessage();

  DpaMessage& operator=(const DpaMessage& other);

  MessageType MessageDirection() const;
  void FillFromResponse(unsigned char* data, uint32_t length);
  void AddDataToBuffer(unsigned char* data, uint32_t length);
  int Length() const { return length_; }
  uint16_t NodeAddress() const { return dpa_packet_->DpaRequestPacket_t.NADR; }
  TDpaPeripheralType PeripheralType() const { return TDpaPeripheralType(dpa_packet_->DpaRequestPacket_t.PNUM); }
  uint8_t CommandCode() const { return dpa_packet_->DpaResponsePacket_t.PCMD; }
  TErrorCodes ResponseCode() const;
  const DpaPacket_t& DpaPacket() const { return *dpa_packet_; }

  unsigned char* DpaPacketData();

 private:
  DpaPacket_t* dpa_packet_;
  const int kCommandIndex = 0x03;
  const int kStatusCodeIndex = 0x06;
  int length_;

  bool IsConfirmationMessage() const;
};

#endif // !__DPA_MESSAGE

