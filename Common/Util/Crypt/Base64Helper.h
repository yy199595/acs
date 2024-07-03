#pragma once
#include <string>
namespace help
{
    namespace Base64
    {
		extern std::string Encode(const std::string & str);
		extern std::string Decode(const std::string & str);
	}
}