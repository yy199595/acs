
#pragma once
#include<string>

namespace help
{
	namespace Sha256
	{
		extern std::string GetHash(const std::string & str);
		extern std::string GetHash(const char * str, size_t size);
		extern std::string XorString(const std::string & s1, const std::string & s2);
		extern std::string GetHMacHash(const std::string & key, const std::string & text);
	}
}