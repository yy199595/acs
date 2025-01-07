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
		inline void Set(int pos, T & value)
		{
			static_assert(std::is_integral<T>::value, "T must be an integral type");
			value |= (static_cast<T>(1) << pos);
		}
		template<typename T>
		inline T Get(int pos, const T & value)
		{
			static_assert(std::is_integral<T>::value, "T must be an integral type");
			return (value >> pos) & static_cast<T>(1);
		}
	}
}
#endif //APP_BIT_H
