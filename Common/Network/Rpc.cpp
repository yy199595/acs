//
// Created by zmhy0073 on 2022/8/25.
//
#include"Rpc.h"
namespace Tcp
{
    RpcMessage::RpcMessage()
    {
        this->mType = 0;
        this->mSize = 0;
        this->mProto = 0;
        this->mBuffer = nullptr;
    }

    RpcMessage::~RpcMessage()
    {
        if(this->mBuffer != nullptr)
        {
            delete []this->mBuffer;
        }
    }

    int RpcMessage::DecodeHead(std::istream &is)
    {
        is.readsome((char *)&this->mSize, sizeof(int));
        this->mType = is.get();
        this->mProto = is.get();
        return this->mSize;
    }

    bool RpcMessage::DecodeBody(std::istream &is)
    {
        if (this->mBuffer == nullptr)
        {
            this->mBuffer = new char[this->mSize];
        }
        return is.readsome(this->mBuffer, this->mSize) == this->mSize;
    }

    const char *RpcMessage::GetData(int &size) const
    {
        size = this->mSize;
        return this->mBuffer;
    }
}
