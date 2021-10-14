//
// Created by zmhy0073 on 2021/10/14.
//

#include "MessageStream.h"

namespace Sentry
{
    const char * MessageStream::Serialize(size_t & size)
    {
		size = this->mPos;
		size_t head = this->mPos - sizeof(unsigned int);
        memcpy(this->mBuffer, &head, sizeof(unsigned int));
        return this->mBuffer;
    }

    MessageStream & MessageStream::operator<<(DataMessageType type)
    {
        memcpy(this->mBuffer + this->mPos, &type, sizeof(char));
        this->mPos += sizeof(char );
        return *this;
    }

    MessageStream & MessageStream::operator<<(unsigned short value)
    {
        memcpy(this->mBuffer + this->mPos, &value, sizeof(unsigned short));
        this->mPos += sizeof(unsigned short);
        return *this;
    }

    MessageStream & MessageStream::operator << (const Message & message)
    {
		std::string ms = message.SerializeAsString();
        size_t size = message.ByteSizeLong();
        if (message.SerializeToArray(this->mBuffer + this->mPos, TCP_SEND_MAX_COUNT - this->mPos))
        {
            this->mPos += size;
        }
        return *this;
    }
}