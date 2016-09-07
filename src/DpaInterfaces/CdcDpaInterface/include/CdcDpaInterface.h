#ifndef DPALIBRARY_CDCDPAINTERFACE_H
#define DPALIBRARY_CDCDPAINTERFACE_H

#include <memory>
#include "DpaInterface.h"

class CdcDpaInterfaceImpl;
class CdcDpaInterface
	: public DpaInterface {
 public:
  /** Default constructor. */
  CdcDpaInterface();

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
  virtual int RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function);

  /**
   * Opens communication with apwcified device address.
   *
   * @param	device	The device address.
   */
  void Open(std::string device);

  /** Closes communication interface. */
  void Close();

 private:
  std::unique_ptr<CdcDpaInterfaceImpl> cdcDpaInterfaceImpl_;
};

#endif //DPALIBRARY_CDCDPAINTERFACE_H
