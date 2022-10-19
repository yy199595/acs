//
// Created by zmhy0073 on 2022/9/27.
//

#ifndef APP_MESSAGE_H
#define APP_MESSAGE_H
#include"Rpc.h"
#include<string>
#include<vector>
#include"XCode/XCode.h"
#include<unordered_map>
#include"Log/CommonLogDef.h"
#include"Proto/ProtoHelper.h"
#include"Message/ProtoMessage.h"

namespace Rpc
{
    class Head : protected std::unordered_map<std::string, std::string>
    {
    public:
        bool Has(const std::string &key) const;

        bool Get(const std::string &key, int &value) const;

        bool Get(const std::string &key, long long &value) const;

        bool Get(const std::string &key, std::string &value) const;

        bool Get(std::vector<std::string> & keys) const;

    public:
        size_t GetLength() const;

        size_t Parse(std::istream &os);

        bool Serialize(std::ostream &os) const;

    public:
        bool Remove(const std::string &key);

        bool Add(const std::string &key, int value);

        bool Add(const std::string &key, XCode value);

        bool Add(const std::string &key, long long value);

        bool Add(const std::string &key, const std::string &value);
    };

    class Packet : public Tcp::ProtoMessage
    {
    public:
        bool ParseLen(std::istream &os, int & len);

        int Serailize(std::ostream &os) final;

        bool Parse(std::istream &os, size_t size);

        static std::shared_ptr<Packet> New(Tcp::Type type, Tcp::Porto proto);

    public:
        XCode GetCode(XCode code) const;

        Head &GetHead() { return this->mHead; }

        void SetType(Tcp::Type type) { this->mType = (int)type; }

        void SetProto(Tcp::Porto proto) { this->mProto = (int)proto; }

        int GetType() const { return this->mType; }

        int GetProto() const { return this->mProto; }

        void Clear() { this->mBody.clear();}
        void SetContent(const std::string & content);
        size_t GetSize() const { return this->mBody.size(); }
        const std::string & GetBody() const { return this->mBody;}
        void Append(const std::string & data) { this->mBody.append(data); }
        bool GetMethod(std::string &service, std::string &method) const;
    public:
        std::shared_ptr<Packet> Clone();

    public:
        template<typename T>
        bool ParseMessage(T * message);

        template<typename T>
        bool WriteMessage(const T * message);

    private:
        int mLen;
        Head mHead;
        int mType;
        int mProto;
        std::string mBody;
    };

    template<typename T>
    bool Packet::ParseMessage(T * message)
    {
        switch (this->mProto)
        {
            case (int)Tcp::Porto::Protobuf:
                if(message->ParseFromString(this->mBody))
                {
                    this->mBody.clear();
                    return true;
                }
                return false;
            case (int)Tcp::Porto::Json:
                if(Helper::Protocol::FromJson(message, this->mBody))
                {
                    this->mBody.clear();
                    return true;
                }
                return false;
        }
        CONSOLE_LOG_FATAL("proto error parse error");
        return false;
    }

    template<typename T>
    bool Packet::WriteMessage(const T * message)
    {
        this->mBody.clear();
        if(message == nullptr)
        {
            return true;
        }
        switch (this->mProto)
        {
            case (int)Tcp::Porto::Protobuf:
                return message->SerializeToString(&mBody);
            case (int)Tcp::Porto::Json:
                return Helper::Protocol::GetJson(*message, &mBody);
        }
        CONSOLE_LOG_FATAL("proto error write error");
        return false;
    }
}


#endif //APP_MESSAGE_H
