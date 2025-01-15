#pragma once
#include <string>

namespace help
{
	namespace str
	{
		class Stream
		{
		public:
			explicit Stream(size_t size = 256)
			{
				this->stringBuffer.reserve(size);
			}

			explicit Stream(const char* str)
			{
				stringBuffer = str;
			}

			explicit Stream(const std::string& str)
			{
				stringBuffer = str;
			}

			inline Stream& operator<<(int data)
			{
				stringBuffer.append(std::to_string(data));
				return (*this);
			}

			inline Stream& operator<<(unsigned int data)
			{
				stringBuffer.append(std::to_string(data));
				return (*this);
			}

			inline Stream& operator<<(long long data)
			{
				stringBuffer.append(std::to_string(data));
				return (*this);
			}

			inline Stream& operator<<(unsigned long long data)
			{
				stringBuffer.append(std::to_string(data));
				return (*this);
			}

			inline Stream& operator<<(float data)
			{
				stringBuffer.append(std::to_string(data));
				return (*this);
			}

			inline Stream& operator<<(double data)
			{
				stringBuffer.append(std::to_string(data));
				return (*this);
			}

			inline Stream& operator<<(const char* data)
			{
				stringBuffer.append(data);
				return (*this);
			}

			inline Stream& operator<<(const std::string& data)
			{
				stringBuffer.append(data);
				return (*this);
			}

			inline const std::string& Serialize()
			{
				return this->stringBuffer;
			}

			inline inline void Clear()
			{
				this->stringBuffer.clear();
			}

			inline void Pop() { this->stringBuffer.pop_back(); }

		private:
			std::string stringBuffer;
		};
	}
}