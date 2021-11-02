//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPCONTENT_H
#define GAMEKEEPER_HTTPCONTENT_H
#include <ostream>
#include <fstream>
namespace GameKeeper
{
    class HttpContent
    {
    public:
        HttpContent() = default;

        virtual ~HttpContent() = default;

    public:
        virtual size_t GetContentSize() = 0;
        virtual bool GetContent(std::ostream &os) = 0;
    };
}

namespace GameKeeper
{
    class HttpStringContent : public HttpContent
    {
    public:
        explicit HttpStringContent(const std::string & content);
    public:
        size_t GetContentSize() override;
        bool GetContent(std::ostream & os) override;
    private:
        const std::string mContent;
    };
}

namespace GameKeeper
{
    class HttpFileContent : public HttpContent
    {
    public:
        explicit HttpFileContent(const std::string &path);
        ~HttpFileContent() override;
    public:
        size_t GetContentSize() override;

        bool GetContent(std::ostream &os) override;
    private:
		size_t mFileSize;
		size_t mSendSize;
        char mBuffer[2048];
        const std::string mPath;
        std::ifstream mFileStream;
    };
}
#endif //GAMEKEEPER_HTTPCONTENT_H
