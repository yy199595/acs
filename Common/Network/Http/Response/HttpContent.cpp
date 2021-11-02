//
// Created by zmhy0073 on 2021/11/2.
//
#include "HttpContent.h"
#ifdef __DEBUG__
#include <Define/CommonDef.h>
#endif // __DEBUG__

namespace GameKeeper
{
    HttpStringContent::HttpStringContent(const std::string &content)
        : mContent(content)
    {

    }

    bool HttpStringContent::GetContent(std::ostream &os)
    {      
		os.write(mContent.c_str(), mContent.size());
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
		this->mFileSize = 0;
		this->mSendSize = 0;
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
            size_t size = this->mFileStream.read(this->mBuffer, 2048).gcount();
            os.write(this->mBuffer, size);	
#ifdef __DEBUG__
			this->mSendSize += size;
			double process = this->mSendSize / (double)this->mFileSize;
			GKDebugError("send " << this->mPath << " [" << process * 100 << "%]");
#endif
            return this->mFileStream.eof();
        }
        return true;
    }

    size_t HttpFileContent::GetContentSize()
    {
        if(!this->mFileStream.is_open())
        {
            this->mFileStream.open(this->mPath, std::ios::binary);
            if(this->mFileStream.is_open())
            {
                this->mFileSize = this->mFileStream.seekg(0, std::ios_base::end).tellg();
                this->mFileStream.seekg(0, std::ios_base::beg);
            }
        }
        return this->mFileSize;
    }
}