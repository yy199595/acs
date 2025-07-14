#pragma once


#include<string>

namespace help
{
	namespace Math
	{
		template<typename T>
		inline const T& Min(const T& t1, const T& t2)
		{
			return t1 < t2 ? t1 : t2;
		}

		template<typename T>
		inline const T& Max(const T& t1, const T& t2)
		{
			return t1 > t2 ? t1 : t2;
		}

		template<typename T>
		inline const T& Clamp(const T& num, const T& min, const T& max)
		{
			if (num >= min && num <= max)
			{
				return num;
			}
			return num < min ? min : max;
		}

		template<typename T>
		inline const T& MinZero(const T& t1, const T& t2)
		{
			return t1 == 0 ? t2 : t2 == 0 ? t1
										  : t1 < t2 ? t1
													: t2;
		}

		template<typename T>
		inline const T& MaxZero(const T& t1, const T& t2)
		{
			return t1 == 0 ? t2 : t2 == 0 ? t1
										  : t1 > t2 ? t1
													: t2;
		}

		inline bool IsInt(const char * str, size_t size)
		{
			if(str == nullptr || size == 0)
			{
				return false;
			}
			size_t start = 0;
			if (str[0] == '+' || str[0] == '-')
			{
				start = 1;
			}
			for (size_t index = start; index < size; index++)
			{
				if (!std::isdigit(str[index]))
				{
					return false;
				}
			}
			return true;
		}

		inline bool IsFloat(const char * str, size_t size)
		{
			if(str == nullptr || size == 0)
			{
				return false;
			}
			size_t start = 0;
			if (str[0] == '+' || str[0] == '-')
			{
				start = 1;
			}
			for (size_t index = start; index < size; index++)
			{
				if(str[index] == '.')
				{
					if(index == 0)
					{
						return false;
					}
				}
				else if (!std::isdigit(str[index]))
				{
					return false;
				}
			}
			return true;
		}

		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, bool> ToNumber(const std::string& str, T& value)
		{
			if(!IsInt(str.c_str(), str.size()))
			{
				return false;
			}
			value = (T)std::stoll(str);
			return true;
		}

		template<typename T>
		inline std::enable_if_t<std::is_floating_point<T>::value, bool> ToNumber(const std::string& str, T& value)
		{
			if(!IsFloat(str.c_str(), str.size()))
			{
				return false;
			}
			value = (T)std::stod(str);
			return true;
		}
	}
}

// namespace MathHelper