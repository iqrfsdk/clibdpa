#ifndef DPALIBRARY_CDCDPAINTERFACE_H
#define DPALIBRARY_CDCDPAINTERFACE_H

#include <functional>
#include "DpaInterface.h"
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


#endif //DPALIBRARY_CDCDPAINTERFACE_H
