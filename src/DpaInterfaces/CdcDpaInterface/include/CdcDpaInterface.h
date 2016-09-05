#ifndef DPALIBRARY_CDCDPAINTERFACE_H
#define DPALIBRARY_CDCDPAINTERFACE_H

#include <memory>
#include "DpaInterface.h"

class CdcDpaInterfaceImpl;
class CdcDpaInterface
	: public DpaInterface {
 public:
  CdcDpaInterface();
  virtual ~CdcDpaInterface();

  virtual int32_t SendRequest(unsigned char* data, uint32_t length);
  virtual int RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function);

  void RegisterCdcListenerWrapper(void(* fptr)(unsigned char*, uint32_t));
  void CdcListenerWrapper(unsigned char* data, uint32_t length);

  void Open(std::string device);
  void Close();

 private:
  std::unique_ptr<CdcDpaInterfaceImpl> cdcDpaInterfaceImpl_;
};

/*
#include <functional>
#include "cdc/CDCImpl.h"

class CdcDpaInterface
	: public DpaInterface {

 public:

  CdcDpaInterface();

  virtual ~CdcDpaInterface();

  virtual int32_t SendRequest(unsigned char* data, uint32_t length);

  virtual int RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function);

  void Init(CDCImpl* cdc_impl);

  void ReceiveData(unsigned char* data, uint32_t length);

 private:
  CDCImpl* cdc_impl_;
  bool is_initialized_;

  std::function<void(unsigned char*, const uint32_t&)> response_callback_;
};
*/

#endif //DPALIBRARY_CDCDPAINTERFACE_H
