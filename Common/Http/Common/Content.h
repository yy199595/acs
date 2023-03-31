//
// Created by yjz on 2022/10/27.
//

#ifndef APP_CONTENT_H
#define APP_CONTENT_H
#include"httpHead.h"
namespace Http
{
    class IContent : public IStream
    {
    public:
        virtual const char * Type() const = 0;
    };
}

namespace Http
{
    class JsonContent : public IContent
    {
    public:
        JsonContent(const std::string & json);
        JsonContent(const char * json, size_t len);
    public:
        bool OnRead(std::istream & buffer) final;
        int OnWrite(std::ostream & buffer) final;
        const char * Type() const { return Http::ContentName::JSON; }
        const std::string & GetContent() const { return this->mJson; }
    private:
        std::string mJson;
    };
}


#endif //APP_CONTENT_H
