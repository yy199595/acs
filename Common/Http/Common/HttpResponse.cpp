//
// Created by yjz on 2022/10/27.
//

#include"HttpResponse.h"
#include"Util/Json/JsonWriter.h"
#ifdef __DEBUG__
#include"Entity/Unit/App.h"
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
			/*std::string firstLine;
			std::getline(buffer, firstLine);*/

			buffer >> this->mVersion;
			buffer >> this->mCode;		
			std::getline(buffer, this->mError);
			if (!this->mError.empty() && this->mError.back() == '\r')
			{
				this->mError.pop_back();
			}
			this->mParseState = DecodeState::Head;
			return HTTP_READ_LINE;
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
			buffer.read(buff, sizeof(buff));
			size_t size = buffer.gcount();
			while(size > 0)
			{
				count = this->OnReadContent(buff, size);
				if(count <= 0)
				{
					return count;
				}
				buffer.read(buff, sizeof(buff));
				size = buffer.gcount();
			}
			return count;
		}
		return HTTP_READ_ERROR;
	}

	int IResponse::OnWrite(std::ostream& buffer)
	{
		HttpStatus code = this->GetCode();
		if(this->mParseState == DecodeState::None)
		{
			buffer << this->mVersion << ' ' << (int)code << ' '
				   << HttpStatusToString(code) << "\r\n";
			this->mHead.OnWrite(buffer);
			buffer << "\r\n";
			this->mParseState = DecodeState::Body;
		}
		if (code != HttpStatus::OK)
		{
			return 0;
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

	int DataResponse::WriteToLua(lua_State* lua) const
	{
		lua_createtable(lua, 0, 5);
		{
			lua_pushstring(lua, "code");
			lua_pushinteger(lua, this->mCode);
			lua_rawset(lua, -3);
		}
		{
			lua_pushstring(lua, "version");
			lua_pushstring(lua, this->mVersion.c_str());
			lua_rawset(lua, -3);
		}
		{
			lua_pushstring(lua, "status");
			lua_pushstring(lua, this->mError.c_str());
			lua_rawset(lua, -3);
		}
		{
			lua_pushstring(lua, "head");
			std::vector<std::string> keys;
			this->mHead.Keys(keys);
			lua_createtable(lua, 0, keys.size());
			for(const std::string & key : keys)
			{
				std::string value;
				if(this->mHead.Get(key, value))
				{
					lua_pushstring(lua, key.c_str());
					lua_pushstring(lua, value.c_str());
					lua_rawset(lua, -3);
				}
			}
			lua_rawset(lua, -3);
		}
		{
			lua_pushstring(lua, "body");
			lua_pushlstring(lua, this->mContent.c_str(), this->mContent.size());
			lua_rawset(lua, -3);
		}
		return 1;
	}

	int DataResponse::OnReadContent(const char* str, size_t size)
	{
		int length = this->mHead.ContentLength();
		if(length > 0)
		{
			this->mContent.append(str, size);
			return length - (int)this->mContent.size();
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
	FileResponse::FileResponse(const std::string & path)
		: mPath(path)
	{
		this->mCurSize = 0;
		this->mFileSize = 0;
		this->mInput = nullptr;
		this->mOutput = nullptr;
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

	size_t FileResponse::ContentSize()
	{
		if (this->mInput == nullptr)
		{
			this->mInput = new std::ifstream();
			this->mInput->open(this->mPath, std::ios::in | std::ios::binary);
			if (!this->mInput->is_open())
			{
				return HTTP_READ_ERROR;
			}
		}
		if (this->mFileSize == 0)
		{
			this->mInput->seekg(0, std::ios::end);
			this->mFileSize = this->mInput->tellg();
			this->mInput->seekg(0, std::ios::beg);
		}
		return this->mFileSize;
	}

	int FileResponse::WriteToLua(lua_State* lua) const
	{
		lua_pushinteger(lua, this->mCode);
		return 1;
	}
	int FileResponse::OnReadContent(const char* str, size_t size)
	{	
		int length = this->mHead.ContentLength();
		if (this->mOutput == nullptr)
		{
			this->mOutput = new std::ofstream();
			this->mOutput->open(this->mPath, std::ios::ate | std::ios::binary);
			if (!this->mOutput->is_open())
			{
				return HTTP_READ_ERROR;
			}
		}
		this->mCurSize += size;
		this->mOutput->write(str, size);
		if (length > 0)
		{					
			int len = length - (int)this->mCurSize;
			double process = this->mCurSize / (double)length;
#ifdef __DEBUG__
			//CONSOLE_LOG_DEBUG("download file process = [" << process * 100 << "%s]");
#endif
			return len <= 1024 ? len : 1024;
		}		
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
			this->mOutput->flush();
			this->mOutput->close();
			delete this->mOutput;
#ifdef __DEBUG__
			CONSOLE_LOG_DEBUG("download file to disk : " << this->mPath);
#endif
		}
		this->mInput = nullptr;
		this->mOutput = nullptr;
	}

	HttpStatus FileResponse::GetCode()
	{
		if (this->ContentSize() < 0)
		{
			return HttpStatus::INTERNAL_SERVER_ERROR;
		}
		return (HttpStatus)this->mCode;
	}

	int FileResponse::OnWriterContent(std::ostream& buff)
	{
		size_t index = 0;
		size_t readCount = 0;
		 char file[100] = { 0 };
		if (this->mInput == nullptr)
		{
			return HTTP_READ_ERROR;
		}
		do
		{
			index++;
			this->mInput->read(file, sizeof(file));
			readCount = this->mInput->gcount();
			if(readCount > 0)
			{
				buff.write(file, readCount);
				this->mCurSize += readCount;
			}

		/*	double process = this->mCurSize / (double)this->mFileSize;
			CONSOLE_LOG_DEBUG("send process = [" << process * 100 << "%s]");*/

		} while (!this->mInput->eof() && readCount > 0 && index < 20);
		return (int)(this->mFileSize - this->mCurSize);
	}
}