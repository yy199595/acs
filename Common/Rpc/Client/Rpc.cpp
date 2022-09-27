//
// Created by zmhy0073 on 2022/8/25.
//
#include"Rpc.h"
#include"Math/MathHelper.h"
namespace Tcp
{
    BinMessage::BinMessage()
    {
        this->mType = Tcp::Type::None;
        this->mProto = Tcp::Porto::None;
    }

    int BinMessage::DecodeHead(std::istream &is)
    {
        union {
            int len;
            char buf[sizeof(int)];
        } buffer;
        is.readsome(buffer.buf, sizeof(int));

        this->mSize = buffer.len;
        this->mType = (Tcp::Type)is.get();
        this->mProto = (Tcp::Porto)is.get();
        return this->mSize;
    }

    bool BinMessage::DecodeBody(std::istream &is)
    {
        if (this->mType == Tcp::Type::UnitRequest)
        {
            this->mSize -= sizeof(long long);
            union
            {
                long long id;
                char buf[sizeof(long long)];
            } bb;
            is.readsome(bb.buf, sizeof(long long));
        }
        if(this->mSize > 0)
        {
            this->mBuffer.reserve(this->mSize);
        }
        char buffer[128] = {0};
        while (this->mSize > 0)
        {
            size_t len = Helper::Math::Min(
                this->mSize, sizeof(buffer));
            if (is.readsome(buffer, len) != len)
            {
                return false;
            }
            this->mSize -= len;
            this->mBuffer.append(buffer, len);
        }
        return true;
    }

    const char *BinMessage::GetData(int &size) const
    {
        size = this->mBuffer.size();
        return this->mBuffer.c_str();
    }
}

