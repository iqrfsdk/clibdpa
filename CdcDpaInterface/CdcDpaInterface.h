#ifndef DPALIBRARY_CDCDPAINTERFACE_H
#define DPALIBRARY_CDCDPAINTERFACE_H

#include "DpaInterface.h"
#include "CDCImpl.h"

class CdcDpaInterface
	: public DpaInterface {
 public:
  /** Default constructor. */
   CdcDpaInterface(const std::string& portIqrf);

  /** Destructor. */
  virtual ~CdcDpaInterface();

  /**
  Sends a request.

  @param [in,out]	data	Pointer to data to be sent.
  @param	length			The number of bytes to be sent.

  @return	Result of the data send operation. 0 - Data was sent successfully, negative value means
  some error occurred.
  */
  virtual int32_t SendRequest(unsigned char* data, uint32_t length);

  /**
  Registers the response handler, a pointer to function that is called when whole packet is
  received.

  @param [in,out]	function	A pointer to function.

  @return	0 - pointer is stored, bellow 0 some error occurred.
  */
  virtual void RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function);

 private:
   CdcDpaInterface();
   CDCImpl m_cdc;
   std::function<void(unsigned char*, uint32_t)> responseHandlerFunc_;
};

#endif //DPALIBRARY_CDCDPAINTERFACE_H
