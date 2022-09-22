#pragma once
#include <string>

namespace String
{
    class Stream
    {
    public:
        Stream(size_t size = 256) { this->stringBuffer.reserve(size); }

        Stream(const char *str) { stringBuffer = str; }

        Stream(const std::string str) { stringBuffer = str; }

        Stream &operator<<(int data)
        {
            stringBuffer.append(std::to_string(data));
            return (*this);
        }

        Stream &operator<<(unsigned int data)
        {
            stringBuffer.append(std::to_string(data));
            return (*this);
        }

        Stream &operator<<(long long data)
        {
            stringBuffer.append(std::to_string(data));
            return (*this);
        }

        Stream &operator<<(unsigned long long data)
        {
            stringBuffer.append(std::to_string(data));
            return (*this);
        }

        Stream &operator<<(float data)
        {
            stringBuffer.append(std::to_string(data));
            return (*this);
        }

        Stream &operator<<(double data)
        {
            stringBuffer.append(std::to_string(data));
            return (*this);
        }

        Stream &operator<<(const char *data)
        {
            stringBuffer.append(data);
            return (*this);
        }

        Stream &operator<<(const std::string & data)
        {
            stringBuffer.append(data);
            return (*this);
        }

        const std::string &Serialize() { return this->stringBuffer; }

        inline void Clear() { this->stringBuffer = ""; }

    private:
        std::string stringBuffer;
    };
}