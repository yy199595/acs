#pragma once

#include<random>
namespace Helper
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
    inline const typename std::enable_if<IsIntegerType<T>::value, T>::type Random()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<T> dis;
        return dis(gen);
    }

    template<typename T>
    inline const typename std::enable_if<IsIntegerType<T>::value, T>::type Random(const T min, const T max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<T> dis(min, max);
        return dis(gen);
    }

    template<typename T>
    inline const typename std::enable_if<!IsIntegerType<T>::value, T>::type Random()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<T> dis;
        return dis(gen);
    }

    template<typename T>
    inline const typename std::enable_if<!IsIntegerType<T>::value, T>::type Random(const T min, const T max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<T> dis(min, max);
        return dis(gen);
    }

    template<typename T>
    inline T ToNumber(const std::string & str)
    {
        return T();
    }
    template<> inline int ToNumber(const std::string & str)
    {
        return std::stoi(str);
    }

    template<> inline float ToNumber(const std::string & str)
    {
        return std::stof(str);
    }

	template<> inline double ToNumber(const std::string & str)
	{
		return std::stod(str);
	}

	template<> inline long long ToNumber(const std::string & str)
	{
		return std::stoll(str);
	}
}
}// namespace MathHelper