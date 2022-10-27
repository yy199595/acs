//
// Created by yjz on 2022/10/27.
//

#ifndef APP_COMMAN_H
#define APP_COMMAN_H
#include<string>
#include<istream>
#include<ostream>
#include<unordered_map>
#include"Client/Http.h"
namespace Http
{
    enum class State
    {
        None,
        Head,
        Body
    };
    class IStream
    {
    public:
        virtual int OnRead(std::istream & buffer) = 0;
        virtual int OnWrite(std::ostream & buffer) = 0;
    };
}

namespace Http
{
    class Head : public IStream
    {
    public:
        bool Add(const std::string & k, int v);
        bool Add(const std::string & k, const std::string & v);
    public:
        bool Get(const std::string & k, int & v) const;
        bool Get(const std::string & k, std::string & v) const;
    public:
        int OnRead(std::istream & buffer) final;
        int OnWrite(std::ostream & buffer) final;
    private:
        std::unordered_map<std::string, std::string> mHeads;
    };
}

#endif //APP_COMMAN_H
