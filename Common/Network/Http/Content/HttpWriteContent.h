//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPWRITECONTENT_H
#define GAMEKEEPER_HTTPWRITECONTENT_H
#include <ostream>
#include <fstream>
#include <Util/JsonHelper.h>
namespace GameKeeper
{
    class HttpWriteContent
    {
    public:
        HttpWriteContent() = default;

        virtual ~HttpWriteContent() = default;

    public:
        virtual bool WriteBody(std::ostream &os) = 0;
        virtual void WriteHead(std::ostream & is) = 0;
    };
}

namespace GameKeeper
{
    class HttpWriteStringContent : public HttpWriteContent
    {
    public:
        explicit HttpWriteStringContent(const std::string & content);
    public:
        bool WriteBody(std::ostream & os) final;
        void WriteHead(std::ostream &is) final;
    private:
        const std::string mContent;
    };
}

namespace GameKeeper
{
    class HttpWriteFileContent : public HttpWriteContent
    {
    public:
        explicit HttpWriteFileContent(const std::string &path);
        ~HttpWriteFileContent() override;
    public:
        bool WriteBody(std::ostream &os) final;
        void WriteHead(std::ostream &is) final;
    private:
		size_t mFileSize;
#ifdef __DEBUG__
		size_t mSendSize;
#endif
        char mBuffer[1024];
        const std::string mPath;
        std::ifstream mFileStream;
    };
}

namespace GameKeeper
{
    class HttpJsonContent : public HttpWriteContent, public RapidJsonWriter
    {
    public:
        explicit HttpJsonContent() = default;
        ~HttpJsonContent() override = default;
    public:
        bool WriteBody(std::ostream &os) final;
        void WriteHead(std::ostream &os) final;
	private:
		size_t mIndex;
    };
}
#endif //GAMEKEEPER_HTTPWRITECONTENT_H
