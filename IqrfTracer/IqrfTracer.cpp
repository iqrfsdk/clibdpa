/**
 * Copyright 2016-2017 MICRORISC s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "IqrfTrace.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace iqrf {

  class IqrfTracer {
  public:
    static IqrfTracer& getTracer()
    {
      static IqrfTracer t;
      return t;
    }

    IqrfTracer & operator = (const IqrfTracer& t) = delete;
    IqrfTracer(const IqrfTracer& t) = delete;

    bool isOn(TrcLevel level)
    {
      return level <= m_level;
    }

    void write(const std::string& msg)
    {
      std::lock_guard<std::mutex> lck(m_mtx);

      if (m_started) {
        if (m_cout) {
          std::cout << msg;
          std::cout.flush();
        }
        else {
          if (m_ofstream.is_open()) {
            m_ofstream << msg;
            m_ofstream.flush();
            if (m_ofstream.tellp() > m_maxSize)
            {
              resetFile();
            }
          }
        }
      }
    }

    void start(const std::string& fname, TrcLevel level = TrcLevel::Debug, long maxSize = TRC_DEFAULT_FILE_MAXSIZE)
    {
      std::lock_guard<std::mutex> lck(m_mtx);
      closeFile();

      m_fname = fname;
      m_cout = fname.empty();
      m_maxSize = maxSize;
      m_level = level;

      if (!m_fname.empty())
        openFile();

      m_started = true;
    }

    void stop()
    {
      std::lock_guard<std::mutex> lck(m_mtx);
      closeFile();
      m_started = false;
    }

    const char* levelToChar(TrcLevel level)
    {
      switch (level) {
      case TrcLevel::Error:
        return "{ERR}";
      case TrcLevel::Warning:
        return "{WAR}";
      case TrcLevel::Information:
        return "{INF}";
      case TrcLevel::Debug:
        return "{DBG}";
      default:
        return "{???}";
      }
    }

  private:
    IqrfTracer()
      : m_cout(false)
      , m_maxSize(-1)
      , m_started(false)
      , m_level(TrcLevel::Debug)
    {
    }

    void openFile()
    {
      static unsigned count = 0;
      if (!m_ofstream.is_open())
      {
        m_ofstream.open(m_fname, std::ofstream::out | std::ofstream::trunc);
        if (!m_ofstream.is_open())
          std::cerr << std::endl << "Cannot open: " << PAR(m_fname) << std::endl;
        m_ofstream << "file: " << count++ << " opened/reset" << std::endl;
      }
    }

    void closeFile()
    {
      if (m_ofstream.is_open()) {
        m_ofstream.flush();
        m_ofstream.close();
      }
    }

    void resetFile()
    {
      closeFile();
      openFile();
    }

    std::mutex m_mtx;
    std::string m_fname;
    std::ofstream m_ofstream;
    bool m_cout;
    bool m_started;
    long m_maxSize;
    TrcLevel m_level;

  };

  /////////////////////////////////////////
  void tracerStart(const std::string& filename, TrcLevel level, int filesize)
  {
    IqrfTracer::getTracer().start(filename, level, filesize);
  }

  void tracerStop()
  {
    IqrfTracer::getTracer().stop();
  }

  bool tracerIsValid(TrcLevel level, int channel)
  {
    //channel is not supported here
    return IqrfTracer::getTracer().isOn(level);
  }

  void tracerMessage(TrcLevel level, int channel, const char* moduleName,
    const char* sourceFile, int sourceLine, const char* funcName, const std::string & msg)
  {
    std::ostringstream o;

    // write time
    using namespace std::chrono;
    auto nowTimePoint = std::chrono::system_clock::now();
    auto nowTimePointUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTimePoint.time_since_epoch()).count() % 1000000;
    auto time = std::chrono::system_clock::to_time_t(nowTimePoint);
    auto tm = *std::localtime(&time);
    char buf[80];
    strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", &tm);

    // remove path from source file
    std::string fname = sourceFile;
    size_t found = fname.find_last_of("/\\");
    if (std::string::npos != found)
      fname = fname.substr(found + 1);

    // format msg
    o << std::setfill('0') << std::setw(6) << buf << '.' << nowTimePointUs << ' ' <<
      IqrfTracer::getTracer().levelToChar(level) << ' ' << channel << ' ' <<
      moduleName << ' ' << fname << ':' << sourceLine << ' ' << funcName << "() " << std::endl <<
      msg;
    o.flush();

    IqrfTracer::getTracer().write(o.str());
  }

}
