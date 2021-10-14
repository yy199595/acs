//
// Created by zmhy0073 on 2021/10/14.
//

#ifndef SENTRY_MESSAGESTREAM_H
#define SENTRY_MESSAGESTREAM_H
#include <Define/CommonDef.h>
#include <Define/CommonTypeDef.h>

namespace Sentry
{
    enum DataMessageType
    {
        TYPE_NONE,
        TYPE_REQUEST,
        TYPE_RESPONSE,
    };
    class MessageStream
    {
    public:
        MessageStream & operator<<(DataMessageType type);
        MessageStream & operator<<(unsigned short value);
        MessageStream &operator<<(const Message &message);

    public:
        const char *Serialize(size_t &size);

        void ResetPos()
        { this->mPos = sizeof(unsigned int); }

    private:
        size_t mPos;
        char mBuffer[TCP_SEND_MAX_COUNT];
    };
}

#endif //SENTRY_MESSAGESTREAM_H
