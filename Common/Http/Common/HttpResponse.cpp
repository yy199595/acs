//
// Created by yjz on 2022/10/27.
//

#include"HttpResponse.h"
#include"Util/Json/JsonWriter.h"
#ifdef __DEBUG__
#include"Entity/App/App.h"
#endif
namespace Http
{
	IResponse::IResponse()
	{
		this->mVersion = HttpVersion;
		this->mCode = (int)HttpStatus::OK;
		this->mParseState = DecodeState::None;
	}

	int IResponse::OnRead(std::istream& buffer)
	{
		if(this->mParseState == DecodeState::None)
		{
			buffer >> this->mVersion;
			buffer >> this->mCode;
			buffer >> this->mError;
			buffer.ignore(2); //放弃\r\n
			this->mParseState = DecodeState::Head;
		}
		if(this->mParseState == DecodeState::Head)
		{
			if(this->mHead.OnRead(buffer) == HTTP_READ_LINE)
			{
				return HTTP_READ_LINE;
			}
			this->mParseState = DecodeState::Body;
		}
		if(this->mParseState == DecodeState::Body)
		{
			int count = 0;
			char buff[512] = {0};
			size_t size = buffer.readsome(buff, sizeof(buff));
			while(size > 0)
			{
				count = this->OnReadContent(buff, size);
				if(count <= 0)
				{
					return count;
				}
				size = buffer.readsome(buff, sizeof(buff));
			}
			return count;
		}
		return HTTP_READ_ERROR;
	}

	int IResponse::OnWrite(std::ostream& buffer)
	{
		if(this->mParseState == DecodeState::None)
		{
			buffer << this->mVersion << ' ' << this->mCode << ' '
				   << HttpStatusToString((HttpStatus)this->mCode) << "\r\n";
			this->mHead.Add("content-length", (int)this->ContentSize());
			this->mHead.OnWrite(buffer);
			buffer << "\r\n";
			this->mParseState = DecodeState::Body;
		}
		return this->OnWriterContent(buffer);
	}
}

namespace Http
{
	int DataResponse::OnWriterContent(std::ostream& buff)
	{
		const char * str = this->mContent.c_str();
		const size_t size = this->mContent.size();
		buff.write(str, size);
		return 0;
	}

	int DataResponse::OnReadContent(const char* str, size_t size)
	{
		long long length = this->mHead.ContentLength();
		if(length > 0)
		{
			this->mContent.append(str, size);
			return length - this->mContent.size();
		}
		this->mContent.append(str, size);
		return HTTP_READ_SOME;
	}


    void DataResponse::Str(HttpStatus code, const std::string &str)
    {
		this->mCode = (int)code;
        this->mContent.assign(str.c_str(), str.size());
		this->mHead.Add(Http::HeadName::ContentType, Http::ContentName::TEXT);
		this->mHead.Add(Http::HeadName::ContentLength, (int)this->mContent.size());
	}

    void DataResponse::Json(HttpStatus code, Json::Writer& doc)
    {
		this->mCode = (int)code;
		doc.WriterStream(&mContent);
        this->mHead.Add(Http::HeadName::ContentType, Http::ContentName::JSON);
		this->mHead.Add(Http::HeadName::ContentLength, (int)this->mContent.size());
	}

	void DataResponse::Html(HttpStatus code, const std::string& html)
	{
		this->mCode = (int)code;
		this->mContent.assign(html);
		this->mHead.Add(Http::HeadName::ContentType, Http::ContentName::HTML);
		this->mHead.Add(Http::HeadName::ContentLength, (int)this->mContent.size());
	}

    void DataResponse::Json(HttpStatus code, const std::string &json)
    {
       this->Json(code, json.c_str(), json.size());
    }

    void DataResponse::Json(HttpStatus code, const char *str, size_t len)
    {
		this->mCode = (int)code;
		this->mContent.assign(str, len);
        this->mHead.Add(Http::HeadName::ContentType, Http::ContentName::JSON);
		this->mHead.Add(Http::HeadName::ContentLength, (int)this->mContent.size());
	}

    void DataResponse::Content(HttpStatus code, const std::string& type, const std::string& str)
    {
		this->mCode = (int)code;
		this->mContent.assign(str);
        this->mHead.Add(Http::HeadName::ContentType, type);
        this->mHead.Add(Http::HeadName::ContentLength, (int)this->mContent.size());
    }
}

namespace Http
{
	FileResponse::FileResponse(std::ifstream* fs)
		: mInput(fs), mOutput(nullptr)
	{
		this->mCurSize = 0;
		fs->seekg(0, std::ios_base::end);
		this->mFileSize = fs->tellg();
		this->mInput->seekg(0, std::ios::beg);
	}

	FileResponse::FileResponse(std::ofstream* fs)
		: mOutput(fs), mInput(nullptr)
	{
		this->mCurSize = 0;
		this->mFileSize = 0;
	}

	FileResponse::~FileResponse()
	{
		if (this->mInput)
		{
			this->mInput->close();
			delete this->mInput;
		}
		if (this->mOutput)
		{
			this->mOutput->close();
			delete this->mOutput;
		}
	}

	int FileResponse::OnReadContent(const char* str, size_t size)
	{
		assert(this->mOutput);
		int length = this->mHead.ContentLength();
		if (length > 0)
		{
			this->mCurSize += size;
			this->mOutput->write(str, size);
			int len = length - (int)this->mCurSize;
			return len <= 512 ? len : 512;
		}
		this->mCurSize += size;
		this->mOutput->write(str, size);
		return HTTP_READ_SOME;
	}

	void FileResponse::OnComplete()
	{
		if (this->mInput)
		{
			this->mInput->close();
			delete this->mInput;
		}
		if (this->mOutput)
		{
			this->mOutput->close();
			delete this->mOutput;
		}
		this->mInput = nullptr;
		this->mOutput = nullptr;
	}

	int FileResponse::OnWriterContent(std::ostream& buff)
	{
		size_t index = 0;
		size_t readCount = 0;
		constexpr int Length = 100;
		char file[Length] = { 0 };
		do
		{
			index++;
			this->mInput->read(file, Length);
			readCount = this->mInput->gcount();
			if(readCount > 0)
			{
				buff.write(file, readCount);
				this->mCurSize += readCount;
			}

		} while (readCount > 0 && index < 20);
		return (int)(this->mFileSize - this->mCurSize);
	}
}