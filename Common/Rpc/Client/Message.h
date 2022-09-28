//
// Created by zmhy0073 on 2022/9/27.
//

#ifndef APP_MESSAGE_H
#define APP_MESSAGE_H
#include"Rpc.h"
#include<string>
#include"XCode/XCode.h"
#include<unordered_map>
#include"Message/ProtoMessage.h"


namespace Rpc
{
    class Head : protected std::unordered_map<std::string, std::string>
    {
    public:
        bool Has(const std::string & key) const;
        bool Get(const std::string & key, int & value) const;
        bool Get(const std::string & key, long long & value) const;
        bool Get(const std::string & key, std::string & value) const;
    public:
        size_t GetLength();
        size_t Parse(std::istream & os);
        bool Serialize(std::ostream & os);
    public:
        bool Remove(const std::string & key);
        bool Add(const std::string & key, int value);
        bool Add(const std::string & key, long long value);
        bool Add(const std::string & key, const std::string & value);
    };

    class Data : public Tcp::ProtoMessage
    {
    public:
        int ParseLen(std::istream & os);
        int Serailize(std::ostream &os) final;
        bool Parse(std::istream & os, size_t size);
    public:
        XCode GetCode(XCode code) const;
        Head & GetHead() { return this->mHead;}
        std::string * GetBody() { return &mBody; }
        void ClearBody() { this->mBody.clear(); }
        void SetType(Tcp::Type type) { this->mType = type; }
        void SetProto(Tcp::Porto proto) { this->mProto = proto; }
        const Tcp::Type & GetType() const { return this->mType; }
        const Tcp::Porto & GetProto() const { return this->mProto; }
        bool GetMethod(std::string & service, std::string & method) const;
    public:
        template<typename T>
        bool ParseMessage(std::shared_ptr<T> message) const;
    private:
        int mLen;
        Head mHead;
        Tcp::Type mType;
        Tcp::Porto mProto;
        std::string mBody;
    };

    template<typename T>
    bool Data::ParseMessage(std::shared_ptr<T> message) const
    {
        const char * data = this->mBody.c_str();
        const int size = (int)this->mBody.size();
        return message->ParseFromArray(data, size);
    }
}


#endif //APP_MESSAGE_H
