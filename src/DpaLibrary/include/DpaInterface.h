#ifndef __DPA_INTERFACE
#define __DPA_INTERFACE

#include <functional>
#include <cstdint>

class DpaInterface {
 public:
  virtual ~DpaInterface() {
  }

  virtual int32_t SendRequest(unsigned char* data, const uint32_t length) = 0;
  virtual int RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)>) = 0;
};

#endif

