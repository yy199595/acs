// //////////////////////////////////////////////////////////
// sha1.h
// Copyright (c) 2014,2015 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

#pragma once
#include<string>

namespace Helper
{
	namespace Sha1
	{
		extern std::string GetHash(const std::string & str);
		extern std::string GetHash(const char * str, size_t size);
		extern std::string XorString(const std::string & s1, const std::string & s2);
		extern std::string GetHMacHash(const std::string & key, const std::string & text);
	}
}