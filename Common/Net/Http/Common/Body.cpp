//
// Created by yy on 2023/11/19.
//

#include"Data.h"
#include<sstream>
#include"Util/Zip/Zip.h"
#include"Yyjson/Lua/ljson.h"
#include"Util/Math/Math.h"
#include"spdlog/fmt/fmt.h"
#include"Util/File/FileHelper.h"
#include"Lua/Engine/LuaInclude.h"
#include"Util/String/String.h"
//#include"Util/Zip/Zip.h"
#include "Core/Map/HashMap.h"
namespace http
{
    bool FromData::OnDecode()
    {
        std::vector<std::string> result;
        help::Str::Split(this->mContent, '&', result);
        for (const std::string& filed: result)
        {
            size_t pos1 = filed.find('=');
            if (pos1 == std::string::npos)
            {
                return false;
            }
            std::string key = filed.substr(0, pos1);
            std::string val = filed.substr(pos1 + 1);
			if(!key.empty() && !val.empty())
			{
				if (val.find("%2F") != std::string::npos)
				{
					help::Str::ReplaceString(val, "%2F", "/");
				}
				this->mParameters.emplace(key, val);
			}
        }
        return true;
    }

	std::string FromData::ToStr() const
	{
		size_t index = 0;
		std::stringstream ss;
		auto iter = this->mParameters.begin();
		for(; iter != this->mParameters.end(); iter++, index++)
		{
			ss << iter->first << "=" << iter->second;
			if(index != this->mParameters.size() -1)
			{
				ss << "&";
			}
		}
		return ss.str();
	}

	void FromData::WriteToLua(lua_State* lua)
	{
		lua_createtable(lua, 0, (int)this->mParameters.size());
		for(auto iter = this->mParameters.begin(); iter != this->mParameters.end(); iter++)
		{
			const std::string & key = iter->first;
			const std::string & value = iter->second;
			lua_pushlstring(lua, value.c_str(), value.size());
			lua_setfield(lua, -2, key.c_str());
		}
	}

    bool FromData::Add(const std::string &k, int v)
    {
        auto iter = this->mParameters.find(k);
		if(iter != this->mParameters.end())
		{
            return false;
        }
        this->mParameters.emplace(k, std::to_string(v));
        return true;
    }

    bool FromData::Add(const std::string &k, const std::string &v)
    {
        auto iter = this->mParameters.find(k);
        if(iter != this->mParameters.end())
        {
            return false;
        }
        this->mParameters.emplace(k, v);
        return true;
    }

	void FromData::Set(const std::string& k, int v)
	{
		this->mParameters[k] = std::to_string(v);
	}

	void FromData::Set(const std::string& k, const std::string& v)
	{
		this->mParameters[k] = v;
	}

    bool FromData::Get(std::vector<std::string> &keys) const
    {
        if(this->mParameters.empty())
        {
            return false;
        }
        keys.reserve(this->mParameters.size());
        auto iter = this->mParameters.begin();
        for(; iter != this->mParameters.end(); iter++)
        {
            keys.emplace_back(iter->first);
        }
        return true;
    }

    bool FromData::Get(const std::string &key, std::string &value) const
    {
        auto iter = this->mParameters.find(key);
        if(iter == this->mParameters.end())
        {
            return false;
        }
        value = iter->second;
        return true;
    }

	bool FromData::Decode(const std::string& content)
	{
		this->mContent = content;
		return this->OnDecode();
	}

    bool FromData::Get(const std::string& k, int& value) const
    {
        auto iter = this->mParameters.find(k);
        if(iter == this->mParameters.end())
        {
            return false;
        }
        const std::string & str = iter->second;
        return help::Math::ToNumber(str, value);
    }

	bool FromData::Get(const std::string& k, long long& value) const
	{
		auto iter = this->mParameters.find(k);
		if(iter == this->mParameters.end())
		{
			return false;
		}
		const std::string & str = iter->second;
		return help::Math::ToNumber(str, value);
	}

    void FromData::OnWriteHead(std::ostream &os)
    {
        os << http::Header::ContentType << ": " << http::Header::FORM << "\r\n";
    }

    int FromData::OnRecvMessage(std::istream & is, size_t size)
    {
		std::unique_ptr<char[]> buffer(new char[size]);
		size_t count = is.readsome(buffer.get(), size);
        this->mContent.assign(buffer.get(), count);
		if(this->mContent.size() >= 1024)
		{
			return tcp::PacketLong;
		}
		return tcp::ReadSomeMessage;
    }

	std::string FromData::Serialize() const
	{
		size_t index = 0;
		std::stringstream ss;
		auto iter = this->mParameters.begin();
		for(; iter != this->mParameters.end(); iter++, index++)
		{
			ss << iter->first << "=" << iter->second;
			if(index < this->mParameters.size() - 1)
			{
				ss << "&";
			}
		}
		return ss.str();
	}
}

namespace http
{
    int JsonData::OnRecvMessage(std::istream& is, size_t size)
    {
		static thread_local char buffer[512] = { 0 };
		size_t count = is.readsome(buffer, sizeof(buffer));
		while(count > 0)
		{
			this->mJson.append(buffer, count);
			count = is.readsome(buffer, sizeof(buffer));
		}
		return tcp::ReadSomeMessage;
	}

	void JsonData::OnWriteHead(std::ostream& os)
	{
		if(this->mJson.empty())
		{
			this->Decode(this->mJson);
		}
		os << http::Header::ContentType << ": " << http::Header::JSON << "\r\n";
		os << http::Header::ContentLength << ": " << this->mJson.size() << "\r\n";
	}

	void JsonData::Write(json::w::Document& document)
	{
		this->mJson.c_str();
		document.Encode(&this->mJson);
	}

	void JsonData::WriteToLua(lua_State* lua)
	{
		lua::yyjson::write(lua, this->mValue);
	}

    int JsonData::OnWriteBody(std::ostream &os)
    {
		os.write(this->mJson.c_str(), this->mJson.size());
		return 0;
	}
}

namespace http
{
	void TextData::OnWriteHead(std::ostream& os)
	{
		if (this->mDeflate)
		{
			os << http::Header::ContentEncoding << ": deflate" << http::CRLF;
		}
		os << http::Header::ContentType << ": " << this->mConType << "\r\n";
		os << http::Header::ContentLength << ": " << this->mContent.size() << "\r\n";
	}

	void TextData::Append(const std::string& content)
	{
		this->mContent.append(content);
	}

	int TextData::OnWriteBody(std::ostream& os)
	{
		if(!this->mContent.empty())
		{
			//std::cout << this->mContent << std::endl;
			os.write(this->mContent.c_str(), this->mContent.size());
			return 0;
		}
		return 0;
	}

	void TextData::WriteToLua(lua_State* lua)
	{
		lua_pushlstring(lua, this->mContent.c_str(), this->mContent.size());
	}

	int TextData::OnRecvMessage(std::istream& is, size_t size)
	{
		static thread_local char buffer[512] = { 0 };
		size_t count = is.readsome(buffer, sizeof(buffer));
		while(count > 0)
		{
			this->mContent.append(buffer, count);
			count = is.readsome(buffer, sizeof(buffer));
			if(this->mContent.size() >= this->mMaxSize)
			{
				return tcp::PacketLong;
			}
		}
		return tcp::ReadSomeMessage;
	}

	void TextData::SetContent(const std::string& type, const std::string& content)
	{
		this->mConType = type;
		this->mContent = content;
	}

	void TextData::SetContent(const std::string& type, const char* content, size_t size)
	{
		this->mConType = type;
		if (size >= 2048)
		{
			if (help::zip::CompressData(content, size, this->mContent))
			{
				this->mDeflate = true;
				return;
			}
		}
		this->mDeflate = false;
		this->mContent.assign(content, size);
	}
}

namespace http
{
	FileData::FileData()
	{
		this->mFileSize = 0;
		this->mSendSize = 0;
		this->mMaxSize = 0;
		this->mType = http::Header::Bin;
	}

	FileData::FileData(std::string t)
		: mType(std::move(t))
	{
		this->mFileSize = 0;
		this->mSendSize = 0;
		this->mMaxSize = 0;
	}

	FileData::~FileData()
	{
		if(this->mFile.is_open())
		{
			this->mFile.close();
		}
	}

	bool FileData::OnDecode()
	{
		if(this->mFile.is_open())
		{
			this->mFile.close();
		}
		return true;
	}

	void FileData::WriteToLua(lua_State* l)
	{
		lua_pushlstring(l, this->mPath.c_str(), this->mPath.size());
	}

	void FileData::OnWriteHead(std::ostream& os)
	{
		os << http::Header::ContentType << ": " << this->mType << "\r\n";
		os << http::Header::ContentLength << ": " << this->mFileSize << "\r\n";
	}

	int FileData::OnRecvMessage(std::istream& is, size_t size)
	{
		std::unique_ptr<char []> buff(new char[size]);
		size_t count = is.readsome(buff.get(), sizeof(buff));
		if(count > 0)
		{
			this->mFileSize += count;
			this->mFile.write(buff.get(), count);
			this->mFile.flush();
		}
		return tcp::ReadSomeMessage;
	}

	bool FileData::MakeFile(const std::string& path)
	{
		this->mFile.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
		if(!this->mFile.is_open())
		{
			return false;
		}
		this->mPath = path;
		return true;
	}

	bool FileData::OpenFile(const std::string& path)
	{
		this->mFile.open(path, std::ios::in | std::ios::binary);
		if(!this->mFile.is_open())
		{
			return false;
		}
		this->mPath = path;
		help::fs::GetFileSize(this->mFile, this->mFileSize);
		return true;
	}

	bool FileData::OpenFile(const std::string& path, const std::string& type)
	{
		this->mType = type;
		this->mPath = path;
		return this->OpenFile(path);
	}

	int FileData::OnWriteBody(std::ostream& os)
	{
		thread_local static char buff[1024] = { 0 };
		size_t size = this->mFile.read(buff, sizeof(buff)).gcount();
		if(size > 0)
		{
			os.write(buff, size);
			this->mSendSize += size;
			return this->mFileSize - this->mSendSize;
		}
		return 0;
	}
}

namespace http
{
	bool TransferData::OpenFile(const std::string& path, const std::string & t)
	{
		this->mType = t;
		this->mPath = path;
		this->mFile.open(path, std::ios::in);
		return this->mFile.is_open();
	}

	void TransferData::WriteToLua(lua_State* l)
	{
		lua_pushlstring(l, this->mPath.c_str(), this->mPath.size());
	}

	bool TransferData::OnDecode()
	{
		this->mFile.flush();
		this->mFile.close();
		return true;
	}

	int TransferData::OnRecvMessage(std::istream& buffer, size_t size)
	{
		if (this->mContSize == 0)
		{
			std::string lineData;
			if(!std::getline(buffer, lineData))
			{
				return tcp::ReadDecodeError;
			}
			if (lineData.empty())
			{
				return tcp::ReadOneLine;
			}
			if(!lineData.empty() && lineData.back() == '\r')
			{
				lineData.pop_back();
			}
			this->mContSize = std::stoul(lineData, nullptr, 16);
			if (this->mContSize == 0)
			{
				return tcp::ReadDone;
			}
			return this->mContSize + 2;
		}
		else
		{
			std::unique_ptr<char[]> readBuffer(new char[this->mContSize]);
			int count = (int)buffer.readsome(readBuffer.get(), this->mContSize);
			{
				this->mFile.write(readBuffer.get(), count);
				this->mFile.flush();
			}
			buffer.ignore(2);
			this->mContSize = 0;
			return tcp::ReadOneLine;
		}
	}

	void TransferData::OnWriteHead(std::ostream& os)
	{
		//os << http::Header::TransferEncoding << ": chunked" << http::CRLF;
		os << http::Header::ContentType << fmt::format(": {}; charset=utf-8\r\n", this->mType);
	}

	int TransferData::OnWriteBody(std::ostream& os)
	{
		thread_local static char input[2048] = { 0 };
		size_t size = this->mFile.read(input, sizeof(input)).gcount();
		if (size > 0)
		{
			os << std::hex << size << "\r\n";
			os.write(input, size) << "\r\n";
			if (!this->mFile.eof())
			{
				return 1;
			}
		}
		os  << "0\r\n\r\n";
		this->mFile.close();
		return 0;
	}
}

namespace http
{
	MultipartFromData::MultipartFromData()
		: mDone(false), mMaxCount(1024 * 1024 * 5), mReadCount(0)
	{

	}

	bool MultipartFromData::OnDecode()
	{
		if(!this->mFile.is_open())
		{
			this->mDone = false;
			return false;
		}
		this->mDone = true;
		this->mFile.close();
		return true;
	}

	void MultipartFromData::WriteToLua(lua_State* l)
	{
		lua_pushlstring(l, this->mPath.c_str(), this->mPath.size());
	}

	void MultipartFromData::Init(const std::string& dir,  const std::string & name, size_t limit)
	{
		this->mDir = dir;
		this->mReadCount = 0;
		this->mMaxCount = limit;
		this->mFileName = name;
	}

	std::string MultipartFromData::ToStr() const
	{
		json::w::Document document;
		document.Add("path", this->mPath);
		document.Add("name", this->mFileName);
		document.Add("size", this->mReadCount);
		document.Add("type", this->mContType);
		return document.JsonString();
	}

	int MultipartFromData::OnWriteBody(std::ostream& os)
	{

		return 0;
	}

	int MultipartFromData::OnRecvMessage(std::istream& buffer, size_t size)
	{
		if(this->mMaxCount > 0 && this->mReadCount >= this->mMaxCount)
		{
			return tcp::ReadDecodeError;
		}
		if(this->mEndBoundy.empty())
		{
			if (!std::getline(buffer, this->mEndBoundy))
			{
				return tcp::ReadOneLine;
			}
			this->mReadCount += this->mEndBoundy.size() + 1;
			if (this->mEndBoundy.back() == '\r')
			{
				this->mEndBoundy.pop_back();
			}
			this->mEndBoundy += "--";
			return tcp::ReadOneLine;
		}
		if (this->mPath.empty())
		{
			std::string line;
			while (std::getline(buffer, line))
			{
				this->mReadCount += (line.size() + 1);
				if (line == "\r")
				{
					break;
				}
				if (line.back() == '\r')
				{
					line.pop_back();
				}
				size_t pos = line.find(':');
				if (pos != std::string::npos)
				{
					std::string key = line.substr(0, pos);
					std::string val = line.substr(pos + 1);
					this->mHeader.Add(key, val);
				}
			}

			std::string fileInfo;
			if(!this->mHeader.Get(http::Header::ContentType, this->mContType))
			{
				return tcp::ReadDecodeError;
			}
			if (!this->mHeader.Get(http::Header::ContentDisposition, fileInfo))
			{
				return tcp::ReadDecodeError;
			}
			size_t pos = this->mContType.find('/');
			if(pos == std::string::npos)
			{
				return tcp::ReadDecodeError;
			}
			std::string type = this->mContType.substr(pos + 1);
			this->mFileName = fmt::format("{}.{}", this->mFileName, type);
			this->mPath = fmt::format("{}/{}", this->mDir, this->mFileName);
			this->mFile.open(this->mPath, std::ios::out | std::ios::trunc | std::ios::binary);
			if (!this->mFile.is_open())
			{
				return tcp::ReadDecodeError;
			}
			return tcp::ReadSomeMessage;
		}

		std::unique_ptr<char[]> lienData(new char[size]);
		size_t count = buffer.readsome(lienData.get(), size);
		if(count > 0)
		{
			//std::cout << "count = " << count << std::endl;
			if(strstr(lienData.get(), this->mEndBoundy.c_str()) != nullptr)
			{
				return tcp::ReadDone;
			}
			this->mReadCount+= count;
			this->mFile.write(lienData.get(), count);
			this->mFile.flush();
		}
		return tcp::ReadOneLine;
	}

}