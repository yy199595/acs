//
// Created by zmhy0073 on 2021/11/2.
//
#include "Network/Http/Content/HttpWriteContent.h"
#ifdef __DEBUG__
#include <Define/CommonDef.h>
#endif // __DEBUG__

namespace GameKeeper
{
    HttpWriteStringContent::HttpWriteStringContent(const std::string &content)
        : mContent(content)
    {

    }

    bool HttpWriteStringContent::GetContent(std::ostream &os)
    {      
		os.write(mContent.c_str(), mContent.size());
        return true;
    }

    size_t HttpWriteStringContent::GetContentSize()
    {
        return mContent.size();
    }
}

namespace GameKeeper
{
    HttpWriteFileContent::HttpWriteFileContent(const std::string &path)
        : mPath(path)
    {
		this->mFileSize = 0;
		this->mSendSize = 0;
    }

    HttpWriteFileContent::~HttpWriteFileContent() noexcept
    {
        if(this->mFileStream.is_open())
        {
            this->mFileStream.close();
        }
    }


    bool HttpWriteFileContent::GetContent(std::ostream &os)
    {
        if(this->mFileStream.is_open())
        {
            size_t size = this->mFileStream.read(this->mBuffer, 1024).gcount();
            os.write(this->mBuffer, size);
#ifdef __DEBUG__
//            this->mSendSize += size;
//            size_t size1 = this->mSendSize / 1024;
//            size_t size2 = this->mFileSize / 1024;
//            double process = this->mSendSize / (double) this->mFileSize;
//            GKDebugError("send " << this->mPath << "[" << size1 << "kb/" << size2
//                                 << "kb] [" << process * 100 << "%]");
        if(this->mFileStream.eof())
        {
            double mb = this->mFileSize / (1024.0f * 1024);
            GKDebugLog("send file " << this->mPath << " successful " << " size = " << mb << "mb");
            return true;
        }
#endif
            return this->mFileStream.eof();
        }
        return true;
    }

    size_t HttpWriteFileContent::GetContentSize()
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

namespace GameKeeper
{
    size_t HttpJsonContent::GetContentSize()
    {
        return this->GetJsonSize();
    }

    bool HttpJsonContent::GetContent(std::ostream &os)
    {
        return this->WriterToStream(os);
    }
}