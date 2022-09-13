//
// Created by zmhy0073 on 2021/10/14.
//

#ifndef GameKeeper_MESSAGESTREAM_H
#define GameKeeper_MESSAGESTREAM_H
#include <Define/CommonLogDef.h>
#include <Define/CommonTypeDef.h>

namespace Tcp
{
    enum class Type
    {
        None,
        Request,
        Response
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
        int GetType() const { return this->mType; }
        int GetProto() const { return this->mProto; }
        const char * GetData(int & size) const;
    private:
        int mSize;
        int mType;
        int mProto;
        char * mBuffer;
    };
}

constexpr int RPC_PACK_HEAD_LEN = sizeof(int) + sizeof(char) + sizeof(char);

#endif //GameKeeper_MESSAGESTREAM_H
