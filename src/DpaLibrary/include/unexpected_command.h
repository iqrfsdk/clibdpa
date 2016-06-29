#ifndef __UNEXPECTED_COMMAND
#define __UNEXPECTED_COMMAND

#include <stdexcept>

class unexpected_command
	: public std::logic_error
{
public:
	explicit unexpected_command(const std::string& _Message)
		: logic_error(_Message)
	{
	}

	explicit unexpected_command(const char* _Message)
		: logic_error(_Message)
	{
	}
};
#endif // !__UNEXPECTED_COMMAND
