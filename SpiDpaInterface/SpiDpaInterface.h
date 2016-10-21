#ifndef DPALIBRARY_SPIDPAINTERFACE_H
#define DPALIBRARY_SPIDPAINTERFACE_H

#include "DpaInterface.h"
#include "spi_iqrf.h"
#include "sysfs_gpio.h"
#include <mutex>
#include <thread>
#include <atomic>

class SpiDpaInterface
	: public DpaInterface {
 public:

  /** Default constructor. */
  SpiDpaInterface(const std::string& portIqrf);

  /** Destructor. */
  virtual ~SpiDpaInterface();

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

  //set/get SPI speed
  void setCommunicationMode(_spi_iqrf_CommunicationMode mode) const;
  _spi_iqrf_CommunicationMode getCommunicationMode() const;

 private:
   SpiDpaInterface();
   std::function<void(unsigned char*, uint32_t)> m_receiveFromFunc;

   std::atomic_bool m_runListenThread;
   std::thread m_listenThread;
   void listen();

   std::string m_port;

   unsigned char* m_rx;
   unsigned m_bufsize;

   std::mutex m_commMutex;
};

class SpiChannelException : public std::exception {
public:
  SpiChannelException(const std::string& cause)
    :m_cause(cause)
  {}

  //TODO ?
#ifndef WIN32
  virtual const char* what() const noexcept(true)
#else
  virtual const char* what() const
#endif
  {
    return m_cause.c_str();
  }

  virtual ~SpiChannelException()
  {}

protected:
  std::string m_cause;
};

#endif //DPALIBRARY_SPIDPAINTERFACE_H
