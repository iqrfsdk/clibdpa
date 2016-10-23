#ifndef __DPA_INTERFACE
#define __DPA_INTERFACE

#include <functional>
#include <cstdint>

/**
 @class	DpaInterface

 @brief	An interface between DPA Library and IQRF communication protocol on lower level.
 */
class DpaInterface {
 public:

  /** Virtual destructor. */
  virtual ~DpaInterface() {
  }

  /**
   Sends a request.
  
   @param [in,out]	data	Pointer to data to be sent.
   @param	length			The number of bytes to be sent.
  
   @return	Result of the data send operation. 0 - Data was sent successfully, negative value means some error
   			occurred.
   */
  virtual int32_t SendRequest(const unsigned char* data, uint32_t length) = 0;

  /**
   Registers the response handler, a pointer to function that is called when whole packet is received.
  
   @param [in,out]	function	A pointer to function.
  
   @return	0 - pointer is stored, bellow 0 some error occurred.
   */
  virtual void RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function) = 0;
};

#endif

