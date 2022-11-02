//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPHEAD_H
#define APP_HTTPHEAD_H
#include<string>
#include<istream>
#include<ostream>
#include<unordered_map>
#include"Client/Http.h"
#include"Message/ProtoMessage.h"
namespace Http
{
    enum class DecodeState
    {
        None,
        Head,
        Body
    };
    class IStream
    {
    public:
        virtual bool OnRead(std::istream & buffer) = 0; //true 解析完成 false 解析失败
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
        bool Get(const std::string & k, std::string & v) const;
    public:
        bool OnRead(std::istream & buffer) final;
        int OnWrite(std::ostream & buffer) final;
    private:
        std::unordered_map<std::string, std::string> mHeads;
    };
}

#endif //APP_HTTPHEAD_H
