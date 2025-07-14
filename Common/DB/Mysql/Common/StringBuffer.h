//
// Created by 64658 on 2025/6/9.
//

#pragma once;
#include <string>
#include <vector>

namespace util
{
	class StringBuffer
	{
	public:
		explicit StringBuffer(size_t size = 128) { buffer.reserve(size); }
	public:
		template<typename T>
		inline std::enable_if_t<std::is_integral<T>::value, StringBuffer&> operator<<(T number)
		{
			buffer += std::to_string(number);
			return *this;
		}

		template<typename T>
		inline std::enable_if_t<std::is_floating_point<T>::value, StringBuffer&> operator<<(T number)
		{
			buffer += std::to_string(number);
			return *this;
		}

	public:
		 inline StringBuffer& operator<<(const std::string& str)
		{
			buffer.append(str);
			return *this;
		}

		inline StringBuffer& operator<<(const char* str)
		{
			buffer.append(str);
			return *this;
		}

		inline StringBuffer& Write(const char* str, size_t size)
		{
			buffer.append(str, size);
			return *this;
		}

		inline StringBuffer & operator << (char cc)
		{
			buffer += cc;
			return *this;
		}

		inline StringBuffer& Concat(const std::vector<std::string>& list, char cc)
		{
			for (size_t index = 0; index < list.size(); index++)
			{
				buffer.append(list.at(index));
				if (index < list.size() - 1)
				{
					buffer += cc;
				}
			}
			return *this;
		}
		inline StringBuffer& Concat(const std::vector<const char*>& list, char cc)
		{
			for (size_t index = 0; index < list.size(); index++)
			{
				buffer.append(list.at(index));
				if (index < list.size() - 1)
				{
					buffer += cc;
				}
			}
			return *this;
		}
	public:
		inline void Clear() { this->buffer.clear(); }
		inline const std::string & ToString() const { return this->buffer; }
	private:
		std::string buffer;
	};
}

