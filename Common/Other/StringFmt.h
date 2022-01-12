//
// Created by zmhy0073 on 2022/1/11.
//

#ifndef GAMEKEEPER_STRINGFMT_H
#define GAMEKEEPER_STRINGFMT_H
#include<string>
#include<google/protobuf/message.h>
#include<google/protobuf/util/json_util.h>

using namespace google::protobuf;

namespace Helper
{
    class Fmt
    {
    public:
        template<typename ... Args>
        static const std::string &Fotmat(std::string &str, Args &&... args)
        {
            str.clear();
            return Fmt::Push(str, std::forward<Args>(args)...);
        }

    private:
        template<typename T, typename ... Args>
        static const std::string &Push(std::string &str, const T &val, Args &&... args)
        {
            Fmt::AddValue(str, val);
            Fmt::Push(str, std::forward<Args>(args)...);
            return str;
        }

        static const std::string &Push(std::string &str) { return str; }

    private:
        static void AddValue(std::string &str, const char value)
        { str += value; }

        static void AddValue(std::string &str, const char *value)
        { str.append(value); }

        static void AddValue(std::string &str, const std::string &value)
        { str.append(value); }

        static void AddValue(std::string &str, const int value)
        { str.append(std::to_string(value)); }

        static void AddValue(std::string &str, const float value)
        { str.append(std::to_string(value)); }

        static void AddValue(std::string &str, const short value)
        { str.append(std::to_string(value)); }

        static void AddValue(std::string &str, const double value)
        { str.append(std::to_string(value)); }

        static void AddValue(std::string &str, const long long value)
        { str.append(std::to_string(value)); }

        static void AddValue(std::string &str, const unsigned int value)
        { str.append(std::to_string(value)); }

        static void AddValue(std::string &str, const unsigned short value)
        { str.append(std::to_string(value)); }

        static void AddValue(std::string &str, const unsigned long value)
        { str.append(std::to_string(value)); }

        static void AddValue(std::string &str, const unsigned long long value)
        { str.append(std::to_string(value)); }
    };
}
#endif //GAMEKEEPER_STRINGFMT_H
