//
// Created by zmhy0073 on 2022/9/27.
//

#ifndef APP_MESSAGE_H
#define APP_MESSAGE_H
#include"Rpc.h"
#include<string>
#include<unordered_map>
#include"Message/ProtoMessage.h"


namespace Rpc
{
    class Head : protected std::unordered_map<std::string, std::string>
    {
    public:
        bool Get(const std::string & key, int & value);
        bool Get(const std::string & key, long long & value);
        bool Get(const std::string & key, std::string & value);
    public:
        size_t Parse(std::istream & os);
        bool Serialize(std::ostream & os);
        size_t GetLength() const { return this->mLen; }
    public:
        bool Remove(const std::string & key);
        bool Add(const std::string & key, int value);
        bool Add(const std::string & key, long long value);
        bool Add(const std::string & key, const std::string & value);
    private:
        size_t mLen;
    };

    class Data : public Tcp::ProtoMessage
    {
    public:
        int ParseLen(std::istream & os);
        int Serailize(std::ostream &os) final;
        bool Parse(std::istream & os, size_t size);
    public:
        Head & GetHead() { return this->mHead;}
        std::string & GetBody() { return this->mBody; }
        void SetType(Tcp::Type type) { this->mType = type; }
        void SetProto(Tcp::Porto proto) { this->mProto = proto; }
        const Tcp::Type & GetType() const { return this->mType; }
        const Tcp::Porto & GetProto() const { return this->mProto; }
    private:
        int mLen;
        Head mHead;
        Tcp::Type mType;
        Tcp::Porto mProto;
        std::string mBody;
    };
}


#endif //APP_MESSAGE_H
