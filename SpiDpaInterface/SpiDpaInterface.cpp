#include "SpiDpaInterface.h"
//#include "IqrfLogging.h"
//#include "PlatformDep.h"
#include <string.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>

const unsigned SPI_REC_BUFFER_SIZE = 1024;

//TODO replace by common header file
#define TRC_DBG(msg) { std::cout << msg << std::endl; }
#define TRC_ENTER(msg) { std::cout << msg << std::endl; }
#define TRC_INF(msg) { std::cout << msg << std::endl; }
#define TRC_WAR(msg) { std::cout << msg << std::endl; }
#define PAR(par)                #par "=\"" << par << "\" "
#define PAR_HEX(par)            #par "=\"0x" << std::hex << par << std::dec << "\" "
#define NAME_PAR(name, par)     #name "=\"" << par << "\" "
#define NAME_PAR_HEX(name,par)  #name "=\"0x" << std::hex << par << std::dec << "\" "

#define THROW_EX(extype, exmsg) { \
  std::ostringstream ostrex; ostrex << exmsg; \
  TRC_WAR("Throwing " << #extype << ": " << ostrex.str()); \
  extype ex(ostrex.str().c_str()); throw ex; }

SpiDpaInterface::SpiDpaInterface(const std::string& portIqrf)
  :m_port(portIqrf)
  , m_bufsize(SPI_REC_BUFFER_SIZE)
{
  m_rx = new unsigned char[m_bufsize];
  memset(m_rx, 0, m_bufsize);

  int retval = spi_iqrf_init(portIqrf.c_str());
  if (BASE_TYPES_OPER_OK != retval) {
    THROW_EX(SpiChannelException, "Communication interface has not been open.");
  }

  m_runListenThread = true;
  m_listenThread = std::thread(&SpiDpaInterface::listen, this);
}

SpiDpaInterface::~SpiDpaInterface()
{
  m_runListenThread = false;

  TRC_DBG("joining udp listening thread");
  if (m_listenThread.joinable())
    m_listenThread.join();
  TRC_DBG("listening thread joined");

  spi_iqrf_destroy();

  delete[] m_rx;
}

void SpiDpaInterface::RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function) {
  m_receiveFromFunc = function;
}

void SpiDpaInterface::setCommunicationMode(_spi_iqrf_CommunicationMode mode) const
{
  spi_iqrf_setCommunicationMode(mode);
  if (mode != spi_iqrf_getCommunicationMode()) {
    THROW_EX(SpiChannelException, "CommunicationMode was not changed.");
  }
}

_spi_iqrf_CommunicationMode SpiDpaInterface::getCommunicationMode() const
{
  return spi_iqrf_getCommunicationMode();
}

void SpiDpaInterface::listen()
{
  TRC_ENTER("thread starts");

  try {

    //wait for SPI ready
    //    while (m_runListenThread)
    //    {
    //      int recData = 0;
    //
    //      //lock scope
    //      {
    //        std::lock_guard<std::mutex> lck(m_commMutex);
    //
    //        //get status
    //        spi_iqrf_SPIStatus status;
    //        int retval = spi_iqrf_getSPIStatus(&status);
    //        if (BASE_TYPES_OPER_OK != retval) {
    //          THROW_EX(SpiChannelException, "spi_iqrf_getSPIStatus() failed: " << PAR(retval));
    //        }
    //
    //        if (status.dataNotReadyStatus == SPI_IQRF_SPI_READY_COMM) {
    //          break;
    //        }
    //        else {
    //          TRC_DBG(PAR_HEX(status.isDataReady) << PAR_HEX(status.dataNotReadyStatus));
    //
    //        }
    //        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //
    //      }
    //    }

    TRC_INF("SPI is ready");

    while (m_runListenThread)
    {
      int recData = 0;

      //lock scope
      {
        std::lock_guard<std::mutex> lck(m_commMutex);

        //get status
        spi_iqrf_SPIStatus status;
        int retval = spi_iqrf_getSPIStatus(&status);
        if (BASE_TYPES_OPER_OK != retval) {
          //THROW_EX(SpiChannelException, "spi_iqrf_getSPIStatus() failed: " << PAR(retval));
          THROW_EX(SpiChannelException, "spi_iqrf_getSPIStatus() failed: " << PAR(retval));
        }

        if (status.isDataReady) {

          if (status.dataReady > m_bufsize) {
            THROW_EX(SpiChannelException, "Received data too long: " << NAME_PAR(len, status.dataReady) << PAR(m_bufsize));
          }

          //reading 
          int retval = spi_iqrf_read(m_rx, status.dataReady);
          if (BASE_TYPES_OPER_OK != retval) {
            THROW_EX(SpiChannelException, "spi_iqrf_read() failed: " << PAR(retval));
          }
          recData = status.dataReady;
        }
      }

      //unlocked - possible to write in receiveFromFunc
      if (recData && m_receiveFromFunc) {
        std::basic_string<unsigned char> message(m_rx, recData);
        m_receiveFromFunc((unsigned char*)message.data(),message.length());
      }

      //TODO some conditional wait for condition from SPI?
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
  }
  catch (SpiChannelException& e) {
    TRC_WAR("listening thread error: " << e.what());
    m_runListenThread = false;
  }
  TRC_WAR("thread stopped");
}

int32_t SpiDpaInterface::SendRequest(const unsigned char* data, uint32_t length)
{
  static int counter = 0;
  int attempt = 0;
  counter++;

  std::basic_string<unsigned char> message(data, length);
  //TRC_DBG("Sending to IQRF SPI: " << std::endl << FORM_HEX(message.data(), message.size()));

  while (attempt++ < 4) {
    TRC_DBG("Trying to sent: " << counter << "." << attempt);
    //lock scope
    {
      std::lock_guard<std::mutex> lck(m_commMutex);

      //get status
      spi_iqrf_SPIStatus status;
      int retval = spi_iqrf_getSPIStatus(&status);
      if (BASE_TYPES_OPER_OK != retval) {
        THROW_EX(SpiChannelException, "spi_iqrf_getSPIStatus() failed: " << PAR(retval));
      }

      if (status.dataNotReadyStatus == SPI_IQRF_SPI_READY_COMM) {
        int retval = spi_iqrf_write((void*)message.data(), message.size());
        if (BASE_TYPES_OPER_OK != retval) {
          THROW_EX(SpiChannelException, "spi_iqrf_write()() failed: " << PAR(retval));
        }
        break;
      }
      else {
        TRC_DBG(PAR_HEX(status.isDataReady) << PAR_HEX(status.dataNotReadyStatus));
      }
    }
    //wait for next attempt
    TRC_DBG("Sleep for a while ... ");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

