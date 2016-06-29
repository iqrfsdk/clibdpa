#ifndef __UNEXPECTED_PACKET_TYPE
#define __UNEXPECTED_PACKET_TYPE

#include <stdexcept>

class unexpected_packet_type
	: public std::logic_error
{
public:
	explicit unexpected_packet_type(const std::string& _Message)
		: logic_error(_Message)
	{
	}

	explicit unexpected_packet_type(const char* _Message)
		: logic_error(_Message)
	{
	}

};
#endif // !__UNEXPECTED_PACKET_TYPE
