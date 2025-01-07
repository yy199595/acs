#pragma once


#include<string>
#include<algorithm>


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

		inline bool ToNumber(const std::string& str, int& value)
		{
			if (str.empty())
			{
				return false;
			}
			size_t start = 0;
			if (str[0] == '+' || str[0] == '-')
			{
				start = 1;
			}
			for (size_t index = start; index < str.length(); index++)
			{
				if (!std::isdigit(str[index]))
				{
					return false;
				}
			}
			value = std::stoi(str);
			return true;
		}

		inline bool ToNumber(const std::string& str, long long& value)
		{
			if (str.empty())
			{
				return false;
			}
			size_t start = 0;
			if (str[0] == '+' || str[0] == '-')
			{
				start = 1;
			}
			for (size_t index = start; index < str.length(); index++)
			{
				if (!std::isdigit(str[index]))
				{
					return false;
				}
			}
			value = std::stoll(str);
			return true;
		}
	}
}

// namespace MathHelper