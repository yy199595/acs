//
// Created by 64658 on 2025/7/8.
//

#ifndef APP_BLOOMFILTER_H
#define APP_BLOOMFILTER_H
#include <array>
#include <string>
#include "Util/Tools/String.h"
namespace bloom
{
	template<size_t SIZE = 25000>
	class Filter
	{
	public:
		Filter();
	public:
		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, void> Set(T key)
		{
			size_t hash = static_cast<size_t>(key);
			const size_t index = hash % this->mCount;
			unsigned int & value = this->mArray[index];
			{
				size_t pos = hash % this->mSize;
				value |= 1 << pos;
			}
		}
		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, void> Del(T key)
		{
			size_t hash = static_cast<size_t>(key);
			const size_t index = hash % this->mCount;
			unsigned int & value = this->mArray[index];
			{
				size_t pos = hash % this->mSize;
				value &= ~(1 << pos);
			}
		}
		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, bool> Has(T key)
		{
			size_t hash = static_cast<size_t>(key);
			const size_t index = hash % this->mCount;
			unsigned int & value = this->mArray[index];
			{
				size_t pos = hash % this->mSize;
				return ((value >> pos) & 1) == 1;
			}
		}
	public:
		inline void Set(const std::string & key)
		{
			size_t hash = help::Str::Hash(key);
			size_t index = hash % this->mCount;
			unsigned int & value = this->mArray[index];
			{
				size_t pos = hash % this->mSize;
				value |= 1 << pos;
			}
		}
		inline void Del(const std::string & key)
		{
			size_t hash = help::Str::Hash(key);
			size_t index = hash % this->mCount;
			unsigned int & value = this->mArray[index];
			{
				size_t pos = hash % this->mSize;
				value &= ~(1 << pos);
			}
		}
		inline bool Has(const std::string & key)
		{
			size_t hash = help::Str::Hash(key);
			size_t index = hash % this->mCount;
			unsigned int & value = this->mArray[index];
			{
				size_t pos = hash % this->mSize;
				return ((value >> pos) & 1) == 1;
			}
		}
	private:
		size_t mSize;
		size_t mCount;
		std::array<unsigned int, SIZE> mArray;
	};

	template<size_t SIZE>
	Filter<SIZE>::Filter()
	{
		this->mCount = this->mArray.size();
		this->mSize = sizeof(unsigned int) * 8;
	}
}

#endif //APP_BLOOMFILTER_H
