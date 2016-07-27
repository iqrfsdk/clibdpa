#ifndef DPALIBRARY_SPIDPAINTERFACE_H
#define DPALIBRARY_SPIDPAINTERFACE_H

#include <cstdint>

#include "DpaInterface.h"

class SpiDpaInterfaceImpl;
class SpiDpaInterface
	: public DpaInterface {
 public:

  /** Default constructor. */
  SpiDpaInterface();

  /** Destructor. */
  virtual ~SpiDpaInterface();

  /**
   Opens communication with specified device address.
  
   @param	device	The device address.
   */
  void Open(std::string device);

  /** Closes communication interface. */
  void Close();

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
  virtual int32_t RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function);

 private:
  SpiDpaInterfaceImpl* spiDpaInterfaceImpl_;
};

#endif //DPALIBRARY_SPIDPAINTERFACE_H
