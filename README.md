# IQRF - DPA Library

IQRF DPA communication library with basic support of sending and receiving messages in DPA protocol message format. The library is based on DPA.h file where main message structures are defined. Messages are sent via DpaInterface class. DPA Library has no dependency on OS. 
The core of library is DpaHandler class. It contains functions for sending and asynchronously receiving data from communication interface. All DPA commands are composed from request and response for coordinator requests and from request, confirmation and response in case of remote node. All states are internally controlled by DpaHandler class. 
 
Demo application uses implementation of DpaInterface for CDC message parser. CDC message parser is available for Linux and Microsoft Windows OS. 


Features
* compatible with all DCTR modules
* supported programming languages: C++
* supported operating systems: Linux, Windows


Library contains following folders
```
src 						
	DpaLibrary				Source codes of the IQRF DPA library
		include				Folde contains header files for library
	DpaLibraryDemo 			Example of DPA library implementation
```

See [wiki](https://github.com/MICRORISC/iqrfsdk/wiki) for more information.