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
            size_t size = this->mFileStream.read(this->mBuffer, 1024).gcount();
            os.write(this->mBuffer, size);
            return this->mFileStream.eof();
        }
        return true;
    }

    size_t HttpFileContent::GetContentSize()
    {
        size_t  size = 0;
        if(!this->mFileStream.is_open())
        {
            this->mFileStream.open(this->mPath, std::ios::binary);
            if(this->mFileStream.is_open())
            {
                size = this->mFileStream.seekg(0, std::ios_base::end).tellg();
                this->mFileStream.seekg(0, std::ios_base::beg);
            }
        }
        return size;
    }
}