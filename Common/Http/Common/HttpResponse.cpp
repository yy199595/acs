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
		this->mParseState = DecodeState::None;
	}

	bool IResponse::OnRead(std::istream& buffer)
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
			if(!this->mHead.OnRead(buffer))
			{
				return false;
			}
			this->mParseState = DecodeState::Body;
		}
		if(this->mParseState == DecodeState::Body)
		{
			char buff[128] = {0};
			size_t size = buffer.readsome(buff, sizeof(buff));
			while(size > 0)
			{
				if(this->OnReadContent(buff, size))
				{
					return true;
				}
				size = buffer.readsome(buff, sizeof(buff));
			}
		}
		return false;
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
		int count = this->OnWriterContent(buffer);
		return count;
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

	bool DataResponse::OnReadContent(const char* str, size_t size)
	{
		this->mContent.append(str, size);
		return true;
	}


    void DataResponse::Str(HttpStatus code, const std::string &str)
    {
		this->mCode = (int)code;
        this->mContent.assign(str.c_str(), str.size());
		this->mHead.Add("content-type", Http::ContentName::STRING);
		this->mHead.Add("content-length", (int)this->mContent.size());
	}

    void DataResponse::Json(HttpStatus code, Json::Writer& doc)
    {
		this->mCode = (int)code;
		doc.WriterStream(&mContent);
        this->mHead.Add("content-type", Http::ContentName::JSON);
		this->mHead.Add("content-length", (int)this->mContent.size());
	}

	void DataResponse::Html(HttpStatus code, const std::string& html)
	{
		this->mCode = (int)code;
		this->mContent.assign(html);
		this->mHead.Add("content-type", Http::ContentName::HTML);
		this->mHead.Add("content-length", (int)this->mContent.size());
	}

    void DataResponse::Json(HttpStatus code, const std::string &json)
    {
       this->Json(code, json.c_str(), json.size());
    }

    void DataResponse::Json(HttpStatus code, const char *str, size_t len)
    {
		this->mCode = (int)code;
		this->mContent.assign(str, len);
        this->mHead.Add("content-type", Http::ContentName::JSON);
		this->mHead.Add("content-length", (int)this->mContent.size());
	}

    void DataResponse::Content(HttpStatus code, const std::string& type, const std::string& str)
    {
		this->mCode = (int)code;
		this->mContent.assign(str);
        this->mHead.Add("content-type", type);
        this->mHead.Add("content-length", (int)this->mContent.size());
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

	bool FileResponse::OnReadContent(const char* str, size_t size)
	{
		assert(this->mOutput);
		this->mOutput->write(str, size);
		return false;
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
//#ifdef __DEBUG__
//		float process = this->mCurSize / (float )this->mFileSize;
//		CONSOLE_LOG_INFO("send : [" << (int)(process * 100) << "%]");
//#endif
		return (int)(this->mFileSize - this->mCurSize);
	}
}