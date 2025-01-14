//
// Created by 64658 on 2025/1/7.
//

#ifndef APP_BIT_H
#define APP_BIT_H
#include <type_traits>
namespace help
{
	namespace bit
	{
		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, void> Set(int pos, T & value)
		{
			value |= (static_cast<T>(1) << pos);
		}
		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, void> Get(int pos, const T & value)
		{
			return (value >> pos) & static_cast<T>(1);
		}
	}
}
#endif //APP_BIT_H
