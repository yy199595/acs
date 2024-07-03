//
// Created by yjz on 2023/3/2.
//

#include "Hash.h"

namespace help
{
	size_t Hash::String(const std::string& str)
	{
		size_t result = 0;
		for(const char cc : str)
		{
			result = 5 * result + static_cast<size_t>(cc);
		}
		return result;
	}
}