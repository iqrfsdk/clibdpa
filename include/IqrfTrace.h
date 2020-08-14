#pragma once

#include <iomanip>
#include <string>
#include <sstream>

/// init default channel
#ifndef TRC_CHANNEL
#define TRC_CHANNEL 0
#endif

/// init default module name
#ifndef TRC_MNAME
#define TRC_MNAME ""
#endif

#define TRC_START(filename, level, filesize) \
iqrf::tracerStart(filename, level, filesize);

#define TRC_STOP \
iqrf::tracerStop();

/// convenient trace macros
#ifdef _DEBUG
#define TRC_ERROR(msg)          TRC_MSG(iqrf::TrcLevel::Error, TRC_CHANNEL, TRC_MNAME, msg)
#define TRC_WARNING(msg)        TRC_MSG(iqrf::TrcLevel::Warning, TRC_CHANNEL, TRC_MNAME, msg)
#define TRC_INFORMATION(msg)    TRC_MSG(iqrf::TrcLevel::Information, TRC_CHANNEL, TRC_MNAME, msg)
#define TRC_DEBUG(msg)          TRC_MSG(iqrf::TrcLevel::Debug, TRC_CHANNEL, TRC_MNAME, msg)
#define TRC_FUNCTION_ENTER(msg) TRC_MSG(iqrf::TrcLevel::Debug, TRC_CHANNEL, TRC_MNAME, "[ENTER] " << msg)
#define TRC_FUNCTION_LEAVE(msg) TRC_MSG(iqrf::TrcLevel::Debug, TRC_CHANNEL, TRC_MNAME, "[LEAVE] " << msg)

#define TRC_ERROR_CHN(channel, mname, msg)          TRC_MSG(iqrf::TrcLevel::Error, channel, mname, msg)
#define TRC_WARNING_CHN(channel, mname, msg)        TRC_MSG(iqrf::TrcLevel::Warning, channel, mname, msg)
#define TRC_INFORMATION_CHN(channel, mname, msg)    TRC_MSG(iqrf::TrcLevel::Information, channel, mname, msg)
#define TRC_DEBUG_CHN(channel, mname, msg)          TRC_MSG(iqrf::TrcLevel::Debug, channel, mname, msg)
#define TRC_FUNCTION_ENTER_CHN(channel, mname, msg) TRC_MSG(iqrf::TrcLevel::Debug, channel, mname, "[ENTER] " << msg)
#define TRC_FUNCTION_LEAVE_CHN(channel, mname, msg) TRC_MSG(iqrf::TrcLevel::Debug, channel, mname, "[LEAVE] " << msg)

#else
#define TRC_ERROR(msg)          TRC_MSG(iqrf::TrcLevel::Error, TRC_CHANNEL, TRC_MNAME, msg)
#define TRC_WARNING(msg)        TRC_MSG(iqrf::TrcLevel::Warning, TRC_CHANNEL, TRC_MNAME, msg)
#define TRC_INFORMATION(msg)    TRC_MSG(iqrf::TrcLevel::Information, TRC_CHANNEL, TRC_MNAME, msg)
#define TRC_DEBUG(msg)
#define TRC_FUNCTION_ENTER(msg)
#define TRC_FUNCTION_LEAVE(msg)

#define TRC_ERROR_CHN(channel, mname, msg)          TRC_MSG(iqrf::TrcLevel::Error, channel, mname, msg)
#define TRC_WARNING_CHN(channel, mname, msg)        TRC_MSG(iqrf::TrcLevel::Warning, channel, mname, msg)
#define TRC_INFORMATION_CHN(channel, mname, msg)    TRC_MSG(iqrf::TrcLevel::Information, channel, mname, msg)
#define TRC_DEBUG_CHN(channel, mname, msg)
#define TRC_FUNCTION_ENTER_CHN(channel, mname, msg)
#define TRC_FUNCTION_LEAVE_CHN(channel, mname, msg)

#endif

/// exception tracing
#define THROW_EXC_TRC_WAR(extype, exmsg) { \
  TRC_WARNING("Throwing " << #extype << ": " << exmsg); \
  std::ostringstream _ostrex; \
  _ostrex << exmsg; \
  extype _ex(_ostrex.str().c_str()); \
  throw _ex; \
}

#define CATCH_EXC_TRC_WAR(extype, ex, msg) { \
  TRC_WARNING("Caught " << #extype << ": " << ex.what() << std::endl << msg); \
}

/// formats
#define PAR(par)                #par "=\"" << par << "\" "
#define PAR_HEX(par)            #par "=\"0x" << std::hex << par << std::dec << "\" "
#define NAME_PAR(name, par)     #name "=\"" << par << "\" "
#define NAME_PAR_HEX(name,par)  #name "=\"0x" << std::hex << par << std::dec << "\" "

/// auxiliar macro
#define TRC_MSG(level, channel, mname, msg) \
if (iqrf::tracerIsValid(level, channel)) { \
  std::ostringstream _ostrmsg; \
  _ostrmsg << msg << std::endl; \
  iqrf::tracerMessage(level, channel, mname, __FILE__, __LINE__, __FUNCTION__, _ostrmsg.str()); \
}

namespace iqrf {

  static const long TRC_DEFAULT_FILE_MAXSIZE(10 * 1024 * 1024);

  enum TrcLevel {
    Error,
    Warning,
    Information,
    Debug
  };

  // Trace functions declarations
  void tracerStart(const std::string& filename, TrcLevel level, int filesize);
  void tracerStop();
  bool tracerIsValid(TrcLevel level, int channel);
  void tracerMessage(TrcLevel level, int channel, const char* moduleName,
    const char* sourceFile, int sourceLine, const char* funcName, const std::string & msg);

}
