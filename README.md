# DPA Library for Linux and Windows

[![Build Status](https://travis-ci.org/iqrfsdk/clibdpa.svg?branch=master)](https://travis-ci.org/iqrfsdk/clibdpa)
[![Build Status](https://img.shields.io/appveyor/ci/spinarr/clibdpa/master.svg)](https://ci.appveyor.com/project/spinarr/clibdpa)

IQRF DPA communication library with basic support of sending and receiving messages in DPA protocol message format. 
The library is based on DPA.h file where main message structures are defined. DPA messages are sent via IQRF 
interface/channel class defined in the separate [repository](https://github.com/iqrfsdk/cutils) cutils. 

The core of library is DpaHandler class. It contains functions for sending and asynchronously receiving data from 
communication interface. All states are internally controlled by DpaHandler class.

DpaTransaction has been added to allow for better asynchronous processing. There is definition of DpaTask which is
executed as DPA transaction. The implementation is handled via waiting for system condition which leads to effective
code. Check [basic examples](https://github.com/iqrfsdk/clibdpa/tree/master/DpaExamples) for practical usage of the 
DPA transactions. 

**Features:**

*	compatible with all DCTR modules
*	handle timing for DPA messages
*   supported programming languages: C++
*   supported operating systems: Linux, Windows

**Library contains following folders:**

*	Dpa 			Source codes of the IQRF DPA library
*	DpaDemo			Simple demo based on the library implementation
*	DpaExamples		Basic examples showing how to use the library API
*	DpaExtension	Source codes of the IQRF DPA library extension

**Core classes of the library:**

*	DpaHandler
*	DpaTransfer
*	DpaMessage
*	DpaTask
*	DpaTransactionTask
*	DpaRaw
