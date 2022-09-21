//
// Created by zmhy0073 on 2022/8/25.
//
#include"Rpc.h"
namespace Tcp
{
    BinMessage::BinMessage()
    {
        this->mSize = 0;
        this->mBuffer = nullptr;
        this->mType = Tcp::Type::None;
        this->mProto = Tcp::Porto::None;
    }

    BinMessage::~BinMessage()
    {
        if(this->mBuffer != nullptr)
        {
            delete []this->mBuffer;
        }
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
        if (this->mBuffer == nullptr)
        {
            if(this->mType == Tcp::Type::UnitRequest)
            {
                this->mSize -= sizeof(long long);
                union{
                    long long id;
                    char buf[sizeof(long long)];
                } bb;
                is.readsome(bb.buf, sizeof(long long));
            }
            this->mBuffer = new char[this->mSize];
        }
        return is.readsome(this->mBuffer, this->mSize) == this->mSize;
    }

    const char *BinMessage::GetData(int &size) const
    {
        size = this->mSize;
        return this->mBuffer;
    }
}
