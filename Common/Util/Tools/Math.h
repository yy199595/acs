#pragma once

#include<random>
#include<string>
#include<algorithm>
namespace help
{
namespace Math {
    template<typename T>
    struct IsIntegerType {
        constexpr static bool value = false;
    };

    template<>
    struct IsIntegerType<int> {
        constexpr static bool value = true;
    };

    template<>
    struct IsIntegerType<unsigned int> {
        constexpr static bool value = true;
    };

    template<>
    struct IsIntegerType<short> {
        constexpr static bool value = true;
    };

    template<>
    struct IsIntegerType<unsigned short> {
        constexpr static bool value = true;
    };

    template<>
    struct IsIntegerType<long> {
        constexpr static bool value = true;
    };

    template<>
    struct IsIntegerType<long long> {
        constexpr static bool value = true;
    };

    template<>
    struct IsIntegerType<unsigned long long> {
        constexpr static bool value = true;
    };


    template<typename T>
    inline const T &Min(const T &t1, const T &t2)
    {
        return t1 < t2 ? t1 : t2;
    }

    template<typename T>
    inline const T &Max(const T &t1, const T &t2)
    {
        return t1 > t2 ? t1 : t2;
    }

    template<typename T>
    inline const T &Clamp(const T &num, const T &min, const T &max)
    {
        if (num >= min && num <= max) {
            return num;
        }
        return num < min ? min : max;
    }

    template<typename T>
    inline const T &MinZero(const T &t1, const T &t2)
    {
        return t1 == 0 ? t2 : t2 == 0 ? t1
                                      : t1 < t2 ? t1
                                                : t2;
    }

    template<typename T>
    inline const T &MaxZero(const T &t1, const T &t2)
    {
        return t1 == 0 ? t2 : t2 == 0 ? t1
                                      : t1 > t2 ? t1
                                                : t2;
    }

    template<typename T>
    inline typename std::enable_if<IsIntegerType<T>::value, T>::type Random()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<T> dis;
        return dis(gen);
    }
    // ���������
    template<typename T>
    inline typename std::enable_if<IsIntegerType<T>::value, T>::type Random(const T min, const T max)
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
    inline typename std::enable_if<!IsIntegerType<T>::value, T>::type Random()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<T> dis;
        return dis(gen);
    }

    template<typename T>
    inline typename std::enable_if<!IsIntegerType<T>::value, T>::type Random(const T min, const T max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<T> dis(min, max);
        return dis(gen);
    }

	inline bool ToNumber(const std::string & str, int & value)
	{
		if(str.empty())
		{
			return false;
		}
		size_t start = 0;
		if(str[0] == '+' || str[0] == '-')
		{
			start = 1;
		}
		for(size_t index = start; index < str.length(); index++)
		{
			if(!std::isdigit(str[index]))
			{
				return false;
			}
		}
		value = std::stoi(str);
		return true;
	}

	inline bool ToNumber(const std::string & str, long long & value)
	{
		if(str.empty())
		{
			return false;
		}
		size_t start = 0;
		if(str[0] == '+' || str[0] == '-')
		{
			start = 1;
		}
		for(size_t index = start; index < str.length(); index++)
		{
			if(!std::isdigit(str[index]))
			{
				return false;
			}
		}
		value = std::stoll(str);
		return true;
	}
}
}// namespace MathHelper