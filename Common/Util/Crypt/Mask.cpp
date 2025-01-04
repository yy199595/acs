//
// Created by 64658 on 2025/1/4.
//

#include "Mask.h"
namespace help
{
	void Mask::Encode(std::string& data, const std::string& mask)
	{
		size_t len = mask.size();
		for(size_t index = 0; index <data.size(); index++)
		{
			data[index] = data[index]  ^ mask[index % len];
		}
	}

	void Mask::Decode(std::string& data, const std::string& mask)
	{
		size_t len = mask.size();
		for (size_t i = 0; i < data.size(); ++i) {
			data[i] ^= mask[i % len];
		}
	}
}