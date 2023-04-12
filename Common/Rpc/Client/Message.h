//
// Created by zmhy0073 on 2022/9/27.
//

#ifndef APP_MESSAGE_H
#define APP_MESSAGE_H
#include"Rpc.h"
#include<string>
#include<vector>
#include<unordered_map>
#include"Util/Proto/ProtoHelper.h"
#include"Proto//Message/ProtoMessage.h"

namespace Rpc
{
	namespace Net
	{
		constexpr int TCP = 1;
		constexpr int HTTP = 2;
	}
    class Head : protected std::unordered_map<std::string, std::string>
    {
    public:
        bool Has(const std::string &key) const;

        bool Get(const std::string &key, int &value) const;

        bool Get(const std::string &key, long long &value) const;

        bool Get(const std::string &key, std::string &value) const;

        bool Get(std::vector<std::string> & keys) const;

        const std::string& GetStr(const std::string& key) const;
    public:
        size_t GetLength() const;

        size_t Parse(std::istream &os);

        bool Serialize(std::ostream &os) const;

    public:
        bool Remove(const std::string &key);

        bool Add(const std::string &key, int value);

        bool Add(const std::string &key, long long value);

        bool Add(const std::string &key, const std::string &value);
    };

    class Packet : public Tcp::ProtoMessage
    {
    public:
        bool ParseLen(std::istream &os, int & len);

        int Serialize(std::ostream &os) final;

        bool Parse(const std::string & address, std::istream &os, size_t size);

    public:
        int GetCode(int code = 1) const;

        inline Head & GetHead() { return this->mHead; }

		inline const Head & ConstHead() const { return this->mHead; }

        inline void SetType(int type) { this->mType = type; }
        inline void SetProto(int proto) { this->mProto = proto; }

        inline int GetType() const { return this->mType; }
        inline int GetProto() const { return this->mProto; }

		inline int GetNet() const { return this->mNet; }
		inline int SetNet(int net) { return this->mNet = net; }
	public:


        inline void Clear() { this->mBody.clear();}
        void SetContent(const std::string & content);
        inline const std::string& From() const { return this->mFrom; }
        inline size_t GetSize() const { return this->mBody.size(); }
        inline const std::string & GetBody() const { return this->mBody;}
        inline void SetFrom(const std::string& address) { this->mFrom = address; }
        inline void Append(const std::string & data) { this->mBody.append(data); }
        bool GetMethod(std::string &service, std::string &method) const;
    public:
        std::shared_ptr<Packet> Clone() const;
    public:
       
        bool ParseMessage(Message * message);

        bool WriteMessage(const Message* message);

    private:
        int mLen;
		int mNet;
		Head mHead;
        int mType;
        int mProto;
        std::string mFrom;
        std::string mBody;
    };
}
typedef std::shared_ptr<Rpc::Packet> RpcPacket;


#endif //APP_MESSAGE_H
