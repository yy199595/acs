//
// Created by zmhy0073 on 2021/11/2.
//
#include "Http/Content/HttpWriteContent.h"
#include"Core/App.h"
#ifdef __DEBUG__
#include <Define/CommonLogDef.h>
#endif // __DEBUG__
#include <Util/DirectoryHelper.h>
namespace GameKeeper
{
    HttpWriteStringContent::HttpWriteStringContent(const std::string &content)
        : mContent(content)
    {

    }

    bool HttpWriteStringContent::WriteBody(std::ostream &os)
    {      
		os.write(mContent.c_str(), mContent.size());
        return true;
    }

    void HttpWriteStringContent::WriteHead(std::ostream &os)
    {
        size_t size = mContent.size();
        os << "Content-Length: " << size << "\r\n";
        os << "Content-Type: " << "text/plain" << "\r\n";
    }
}

namespace GameKeeper
{
    HttpWriteFileContent::HttpWriteFileContent(const std::string &path)
        : mPath(path)
    {
		this->mFileSize = 0;
#ifdef __DEBUG__
		this->mSendSize = 0;
#endif // __DEBUG__	
    }

    HttpWriteFileContent::~HttpWriteFileContent() noexcept
    {
        if(this->mFileStream.is_open())
        {
            this->mFileStream.close();
        }
    }


    bool HttpWriteFileContent::WriteBody(std::ostream &os)
    {
        if(this->mFileStream.is_open())
        {
            size_t size = this->mFileStream.read(this->mBuffer, 1024).gcount();
            os.write(this->mBuffer, size);
#ifdef __DEBUG__
           /* this->mSendSize += size;
            size_t size1 = this->mSendSize / 1024;
            size_t size2 = this->mFileSize / 1024;
            double process = this->mSendSize / (double) this->mFileSize;
            LOG_ERROR("send " << this->mPath << "[" << size1 << "kb/" << size2
                                 << "kb] [" << process * 100 << "%]");*/
        if(this->mFileStream.eof())
        {
            double mb = this->mFileSize / (1024.0f * 1024);
            LOG_DEBUG("send file {0} successful size = {1}mb", this->mPath, mb);
            return true;
        }
#endif
            return this->mFileStream.eof();
        }
        return true;
    }

    void HttpWriteFileContent::WriteHead(std::ostream & os)
    {
        this->mFileSize = 0;
        if(!this->mFileStream.is_open())
        {
            this->mFileStream.open(this->mPath, std::ios::in | std::ios::binary);
            if (this->mFileStream.is_open())
            {
                this->mFileSize = this->mFileStream.seekg(0, std::ios_base::end).tellg();
                this->mFileStream.seekg(0, std::ios_base::beg);
            }
            os << "Content-Path: " << this->mPath << "\r\n";
            os << "Content-Length: " << this->mFileSize << "\r\n";
            os << "Content-Type:" << "multipart/form-data" << "\r\n";
        }
    }
}

namespace GameKeeper
{
	bool HttpJsonContent::WriteBody(std::ostream &os)
	{
        if(!this->mJsonWriter.IsComplete())
        {
            this->mJsonWriter.EndObject();
        }
        os.write(this->mStringBuf.GetString(), this->mStringBuf.GetLength());
		return true;
	}

    void HttpJsonContent::WriteHead(std::ostream &os)
    {
        if(!this->mJsonWriter.IsComplete())
        {
            this->mJsonWriter.EndObject();
        }
        os << "Content-Type: " << "applocation/json" << "\r\n";
        os << "Content-Length: " << this->mStringBuf.GetLength() << "\r\n";
    }
}