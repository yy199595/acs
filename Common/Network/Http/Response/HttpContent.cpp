//
// Created by zmhy0073 on 2021/11/2.
//
#include "HttpContent.h"

namespace GameKeeper
{
    HttpStringContent::HttpStringContent(const std::string &content)
        : mContent(content)
    {

    }

    bool HttpStringContent::GetContent(std::ostream &os)
    {
        os << mContent;
        return true;
    }

    size_t HttpStringContent::GetContentSize()
    {
        return mContent.size();
    }
}

namespace GameKeeper
{
    HttpFileContent::HttpFileContent(const std::string &path)
        : mPath(path)
    {

    }

    HttpFileContent::~HttpFileContent() noexcept
    {
        if(this->mFileStream.is_open())
        {
            this->mFileStream.close();
        }
    }


    bool HttpFileContent::GetContent(std::ostream &os)
    {
        if(this->mFileStream.is_open())
        {
            size_t size = this->mFileStream.readsome(this->mBuffer, 1024);
            os.write(this->mBuffer, size);
            return this->mFileStream.eof();
        }
        return true;
    }

    size_t HttpFileContent::GetContentSize()
    {
        if(!this->mFileStream.is_open())
        {
            this->mFileStream.open(this->mPath, std::ios::in);
            if(this->mFileStream.is_open())
            {
                this->mFileStream.seekg(0, std::ios_base::end);
                return this->mFileStream.tellg();
            }
        }
        return 0;
    }
}