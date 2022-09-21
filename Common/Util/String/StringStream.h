#pragma once
#include <string>
class StringStream
{
public:
    StringStream() { this->stringBuffer = ""; }

    StringStream(const char *str) { stringBuffer = str; }

    StringStream(const std::string str) { stringBuffer = str; }

    StringStream &operator<<(int data)
    {
        stringBuffer.append(std::to_string(data));
        return (*this);
    }

    StringStream &operator<<(unsigned int data)
    {
        stringBuffer.append(std::to_string(data));
        return (*this);
    }

    StringStream &operator<<(long long data)
    {
        stringBuffer.append(std::to_string(data));
        return (*this);
    }

    StringStream &operator<<(unsigned long long data)
    {
        stringBuffer.append(std::to_string(data));
        return (*this);
    }

    StringStream &operator<<(float data)
    {
        stringBuffer.append(std::to_string(data));
        return (*this);
    }

    StringStream &operator<<(double data)
    {
        stringBuffer.append(std::to_string(data));
        return (*this);
    }

    StringStream &operator<<(const char *data)
    {
        stringBuffer.append(data);
        return (*this);
    }

    StringStream &operator<<(const std::string data)
    {
        stringBuffer.append(data);
        return (*this);
    }

    void operator>>(int &data) { data = std::atoi(this->stringBuffer.c_str()); }

    void operator>>(unsigned int &data) { data = std::atoi(this->stringBuffer.c_str()); }

    void operator>>(long &data) { data = std::atol(this->stringBuffer.c_str()); }

    void operator>>(long long &data) { data = std::atoll(this->stringBuffer.c_str()); }

    void operator>>(float &data) { data = (float) std::atof(this->stringBuffer.c_str()); }

    void operator>>(double &data) { data = std::atof(this->stringBuffer.c_str()); }

   const std::string & Serialize() { return this->stringBuffer; }

    void Clear() { this->stringBuffer = ""; }

private:
    std::string stringBuffer;
};