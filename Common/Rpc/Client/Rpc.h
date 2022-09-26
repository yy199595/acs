//
// Created by zmhy0073 on 2021/10/14.
//

#ifndef GameKeeper_MESSAGESTREAM_H
#define GameKeeper_MESSAGESTREAM_H
#include<stdexcept>
#include<unordered_map>
#include"Log/CommonLogDef.h"
namespace Tcp
{
    enum class Type
    {
        None,
        Request,
        Response,
        UnitRequest
    };
    enum class Porto
    {
        None,
        Json,
        Protobuf
    };

    class BinMessage
    {
    public:
        BinMessage();
        ~BinMessage();
        int DecodeHead(std::istream & is);
        bool DecodeBody(std::istream & is);

    public:
        const char * GetData(int & size) const;
        Tcp::Type GetType() const { return this->mType; }
        Tcp::Porto GetProto() const { return this->mProto; }
        long long GetUnitId() const { return this->mUnitId; }
    private:
        int mSize;
        char * mBuffer;
        Tcp::Type mType;
        Tcp::Porto mProto;
        long long mUnitId;
    };
}

namespace Rpc
{
    class Head : protected std::unordered_map<std::string, std::string>
    {
    public:
        bool Get(const std::string & key, std::string & value);
    public:
        Tcp::Type GetType() const { return this->mType;}
        Tcp::Porto GetProto() const { return this->mProto; }
    public:
        bool Parse(std::istream & os);
        bool Serialize(std::ostream & os);
    public:
        bool Add(const std::string & key, int value);
        bool Add(const std::string & key, long long value);
        bool Add(const std::string & key, const std::string & value);
    private:
        Tcp::Type mType;
        Tcp::Porto mProto;
    };

    class Data
    {
    public:

    public:
        Head mHead;
        std::string mBody;
    };
}

typedef std::logic_error rpc_error;
constexpr int RPC_PACK_HEAD_LEN = sizeof(int) + sizeof(char) + sizeof(char);

#endif //GameKeeper_MESSAGESTREAM_H
