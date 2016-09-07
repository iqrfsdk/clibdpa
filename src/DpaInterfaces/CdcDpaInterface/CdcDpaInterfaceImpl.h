#ifndef DPALIBRARY_CDCDPAINTERFACEIMPL_H
#define DPALIBRARY_CDCDPAINTERFACEIMPL_H

#include <cstdint>
#include <string>
#include <bits/unique_ptr.h>
#include <functional>
#include <mutex>

#include "cdc/CDCImpl.h"

class CdcDpaInterfaceImpl {
 public:
  CdcDpaInterfaceImpl();
  virtual ~CdcDpaInterfaceImpl();

  void Open(std::string device);
  void Close();

  virtual int32_t SendRequest(unsigned char* data, uint32_t length);
  virtual int RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function);

  void CdcListener(unsigned char* data, uint32_t length);

 private:
  bool is_initialized_;
  std::unique_ptr<CDCImpl> cdc_parser_;
  std::function<void(unsigned char*, uint32_t)> response_callback_;
  std::mutex callbackFunctionMutex_;
  static CdcDpaInterfaceImpl* instance_;

  static void CdcListenerWrapper(unsigned char* data, uint32_t length);

  void InitCdcParser(std::string device);
};

#endif //DPALIBRARY_CDCDPAINTERFACEIMPL_H
