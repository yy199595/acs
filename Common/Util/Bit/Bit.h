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
		inline std::enable_if_t<std::is_integral<T>::value, void> Set(size_t pos, T & value, T num)
		{
			value |= (num<< pos);
		}
		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, T> Get(size_t pos, const T & value)
		{
			return (value >> pos) & static_cast<T>(1);
		}
	}
}
#endif //APP_BIT_H
