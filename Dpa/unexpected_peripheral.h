#ifndef __UNEXPECTED_PERIPHERAL
#define __UNEXPECTED_PERIPHERAL

#include <stdexcept>

class unexpected_peripheral
	: public std::logic_error
{
public:
	explicit unexpected_peripheral(const std::string& _Message)
		: logic_error(_Message)
	{
	}

	explicit unexpected_peripheral(const char* _Message)
		: logic_error(_Message)
	{
	}
};
#endif // !__UNEXPECTED_PERIPHERAL
