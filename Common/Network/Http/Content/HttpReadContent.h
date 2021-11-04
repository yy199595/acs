//
// Created by zmhy0073 on 2021/11/3.
//

#ifndef GAMEKEEPER_HTTPREADCONTENT_H
#define GAMEKEEPER_HTTPREADCONTENT_H
#include <Util/JsonHelper.h>
namespace GameKeeper
{
    class HttpReadContent
    {
    public:
        HttpReadContent() = default;
        virtual ~HttpReadContent() = default;

    public:
        virtual void OnReadContent(const char * data, size_t size) = 0;
    };
}

namespace GameKeeper
{
    class HttpReadStringContent : public HttpReadContent
    {
    public:
        explicit HttpReadStringContent(std::string & response);

    public:
        void OnReadContent(const char *data, size_t size) override;
    private:
        std::string & mResponse;
    };
}

namespace GameKeeper
{
    class HttpReadFileContent : public HttpReadContent
    {
    public:
        explicit HttpReadFileContent(const std::string & path);
        ~HttpReadFileContent() override;
    public:
        bool OpenFile();
        const std::string & GetPaht() { return this->mPath;}
        void OnReadContent(const char *data, size_t size) override;
    private:
        const std::string & mPath;
        std::ofstream mFileStream;
    };
}
#endif //GAMEKEEPER_HTTPREADCONTENT_H
