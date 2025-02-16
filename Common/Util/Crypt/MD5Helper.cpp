//
// Created by 64658 on 2025/1/17.
//

#include "MD5Helper.h"
#include "Util/Src/md5.h"
namespace help
{
	std::string md5::GetMd5(const std::string& input)
	{
		return GetMd5(input.c_str(), input.size());
	}

	std::string md5::GetMd5(const char* input, size_t size)
	{
		char buffer[16] = { 0 };
		::md5(input, (int)size, buffer);
		return std::string(buffer, sizeof(buffer));
	}
}