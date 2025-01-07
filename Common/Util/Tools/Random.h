//
// Created by 64658 on 2025/1/7.
//

#ifndef APP_RANDOM_H
#define APP_RANDOM_H

#include<random>
#include<type_traits>
namespace help
{
	namespace Rand
	{
		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, T> Random()
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<T> dis;
			return dis(gen);
		}

		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, T> Random(const T min, const T max)
		{
			if (min == max)
			{
				return min;
			}
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<T> dis(min, max);
			return dis(gen);
		}

		template<typename T>
		inline std::enable_if_t<std::is_floating_point<T>::value, T> Random()
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<T> dis;
			return dis(gen);
		}

		template<typename T>
		inline std::enable_if_t<std::is_floating_point<T>::value, T> Random(const T min, const T max)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<T> dis(min, max);
			return dis(gen);
		}


	}
}
#endif //APP_RANDOM_H
