//
// Created by zmhy0073 on 2022/8/25.
//
#include"Rpc.h"
namespace Rpc
{
    Message::Message()
    {
        this->mType = 0;
        this->mSize = 0;
        this->mProto = 0;
        this->mBuffer = nullptr;
    }

    Message::~Message()
    {
        if(this->mBuffer != nullptr)
        {
            delete []this->mBuffer;
        }
    }

    int Message::DecodeHead(std::istream &is)
    {
        this->mType = is.get();
        this->mProto = is.get();
        union {
            int len;
            char buffer[sizeof(int)];
        } buf;
        is.readsome(buf.buffer, sizeof(int));
        this->mSize = buf.len;
        return this->mSize;
    }

    int Message::DecodeBody(std::istream &is)
    {
        if (this->mBuffer == nullptr)
        {
            this->mBuffer = new char[this->mSize];
        }
        return is.readsome(this->mBuffer, this->mSize);
    }

    const char *Message::GetData(int &size) const
    {
        size = this->mSize;
        return this->mBuffer;
    }
}
