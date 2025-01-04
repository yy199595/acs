//
// Created by 64658 on 2025/1/4.
//

#ifndef APP_MASK_H
#define APP_MASK_H
#include <string>
namespace help
{
	namespace Mask
	{
		extern void Encode(std::string & data, const std::string & mask);
		extern void Decode(std::string & data, const std::string & mask);
	};
}


#endif //APP_MASK_H
