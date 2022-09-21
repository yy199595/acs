//
// Created by zmhy0073 on 2021/10/14.
//

#ifndef GameKeeper_MESSAGESTREAM_H
#define GameKeeper_MESSAGESTREAM_H
#include<stdexcept>
#include<Define/CommonLogDef.h>
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
typedef std::logic_error rpc_error;
constexpr int RPC_PACK_HEAD_LEN = sizeof(int) + sizeof(char) + sizeof(char);

#endif //GameKeeper_MESSAGESTREAM_H
