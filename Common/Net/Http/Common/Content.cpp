//
// Created by yy on 2023/11/19.
//

#include"Content.h"
#include<sstream>
#include"Yyjson/Lua/ljson.h"
#include"Util/Tools/Math.h"
#include"fmt.h"
#include "Util/File/DirectoryHelper.h"
#include"Util/File/FileHelper.h"
#include"Lua/Engine/LuaInclude.h"
#include"Util/Tools/String.h"
#include "Core/Map/HashMap.h"
#include "Util/Crypt/Base64Helper.h"
#include "Util/Tools/Guid.h"

namespace http
{
	std::string UrlDecode(const std::string& url)
	{
		std::ostringstream decoded;
		for (size_t i = 0; i < url.size(); ++i)
		{
			if (url[i] == '%' && i + 2 < url.size())
			{
				std::istringstream hex(url.substr(i + 1, 2));
				int value;
				if (hex >> std::hex >> value)
				{
					decoded << static_cast<char>(value);
					i += 2;
				}
			}
			else if (url[i] == '+')
			{
				decoded << ' ';
			}
			else
			{
				decoded << url[i];
			}
		}
		return decoded.str();
	}


	bool FromContent::OnDecode()
	{
		std::string encoded = UrlDecode(this->mContent);
		{
			std::vector<std::string> result;
			help::Str::Split(encoded, '&', result);
			for (const std::string& filed: result)
			{
				size_t pos1 = filed.find('=');
				if (pos1 == std::string::npos)
				{
					return false;
				}
				std::string key = filed.substr(0, pos1);
				std::string val = filed.substr(pos1 + 1);
				if (!key.empty() && !val.empty())
				{
					if (val.find("%2F") != std::string::npos)
					{
						help::Str::ReplaceString(val, "%2F", "/");
					}
					this->mParameters.emplace(key, val);
				}
			}
		}
		return true;
	}

	std::string FromContent::ToStr() const
	{
		size_t index = 0;
		std::stringstream ss;
		auto iter = this->mParameters.begin();
		for (; iter != this->mParameters.end(); iter++, index++)
		{
			const std::string& key = iter->second;
			if (key == http::query::Permission || key == http::query::UserId || key == http::query::ClubId)
			{
				continue;
			}
			ss << iter->first << "=" << iter->second;
			if (index != this->mParameters.size() - 1)
			{
				ss << "&";
			}
		}
		return ss.str();
	}

	void FromContent::WriteToLua(lua_State* lua)
	{
		lua_createtable(lua, 0, (int)this->mParameters.size());
		for (auto iter = this->mParameters.begin(); iter != this->mParameters.end(); iter++)
		{
			const std::string& key = iter->first;
			const std::string& value = iter->second;
			lua_pushlstring(lua, value.c_str(), value.size());
			lua_setfield(lua, -2, key.c_str());
		}
	}

	bool FromContent::Add(const std::string& k, int v)
	{
		auto iter = this->mParameters.find(k);
		if (iter != this->mParameters.end())
		{
			return false;
		}
		this->mParameters.emplace(k, std::to_string(v));
		return true;
	}

	bool FromContent::Add(const std::string& k, const std::string& v)
	{
		auto iter = this->mParameters.find(k);
		if (iter != this->mParameters.end())
		{
			return false;
		}
		this->mParameters.emplace(k, v);
		return true;
	}

	void FromContent::Set(const std::string& k, int v)
	{
		this->mParameters[k] = std::to_string(v);
	}

	void FromContent::Set(const std::string& k, const std::string& v)
	{
		this->mParameters[k] = v;
	}

	bool FromContent::Get(std::vector<std::string>& keys) const
	{
		if (this->mParameters.empty())
		{
			return false;
		}
		keys.reserve(this->mParameters.size());
		auto iter = this->mParameters.begin();
		for (; iter != this->mParameters.end(); iter++)
		{
			keys.emplace_back(iter->first);
		}
		return true;
	}

	bool FromContent::Get(const std::string& key, std::string& value) const
	{
		auto iter = this->mParameters.find(key);
		if (iter == this->mParameters.end())
		{
			return false;
		}
		value = iter->second;
		return true;
	}

	bool FromContent::Decode(const std::string& content)
	{
		this->mContent = content;
		return this->OnDecode();
	}

	bool FromContent::Get(const std::string& k, int& value) const
	{
		auto iter = this->mParameters.find(k);
		if (iter == this->mParameters.end())
		{
			return false;
		}
		const std::string& str = iter->second;
		return help::Math::ToNumber(str, value);
	}

	bool FromContent::Get(const std::string& k, long long& value) const
	{
		auto iter = this->mParameters.find(k);
		if (iter == this->mParameters.end())
		{
			return false;
		}
		const std::string& str = iter->second;
		return help::Math::ToNumber(str, value);
	}

	void FromContent::OnWriteHead(std::ostream& os)
	{
		os << http::Header::ContentType << ": " << http::Header::FORM << "\r\n";
	}

	int FromContent::OnRecvMessage(std::istream& is, size_t size)
	{
		std::unique_ptr<char[]> buffer(new char[size]);
		size_t count = is.readsome(buffer.get(), size);
		this->mContent.assign(buffer.get(), count);
		if (this->mContent.size() >= 1024)
		{
			return tcp::PacketLong;
		}
		return tcp::ReadSomeMessage;
	}

	std::string FromContent::Serialize() const
	{
		size_t index = 0;
		std::stringstream ss;
		auto iter = this->mParameters.begin();
		for (; iter != this->mParameters.end(); iter++, index++)
		{
			ss << iter->first << "=" << iter->second;
			if (index < this->mParameters.size() - 1)
			{
				ss << "&";
			}
		}
		return ss.str();
	}
}

namespace http
{
	int JsonContent::OnRecvMessage(std::istream& is, size_t size)
	{
		size_t count = 0;
		char buffer[512] = { 0 };
		do
		{
			count = is.readsome(buffer, sizeof(buffer));
			if (count > 0)
			{
				this->mJson.append(buffer, count);
			}
		} while (count > 0);
		return tcp::ReadSomeMessage;
	}

	void JsonContent::OnWriteHead(std::ostream& os)
	{
		if (this->mJson.empty())
		{
			this->Decode(this->mJson);
		}
		os << http::Header::ContentType << ": " << http::Header::JSON << "\r\n";
		os << http::Header::ContentLength << ": " << this->mJson.size() << "\r\n";
	}

	void JsonContent::Write(const json::w::Document& document)
	{
		this->mJson.clear();
		document.Encode(&this->mJson);
	}

	void JsonContent::WriteToLua(lua_State* lua)
	{
		lua::yyjson::write(lua, this->mValue);
	}

	int JsonContent::OnWriteBody(std::ostream& os)
	{
		os.write(this->mJson.c_str(), this->mJson.size());
		return 0;
	}
}

namespace http
{
	int XMLContent::OnRecvMessage(std::istream& is, size_t size)
	{
		size_t count = 0;
		char buffer[512] = { 0 };
		do
		{
			count = is.readsome(buffer, sizeof(buffer));
			if (count > 0)
			{
				this->mXml.append(buffer, count);
			}
		} while (count > 0);
		return tcp::ReadSomeMessage;
	}

	void XMLContent::OnWriteHead(std::ostream& os)
	{
		if (this->mXml.empty())
		{
			this->Encode(this->mXml);
		}
		os << http::Header::ContentType << ": " << http::Header::XML << "\r\n";
		os << http::Header::ContentLength << ": " << this->mXml.size() << "\r\n";
	}

	void XMLContent::WriteToLua(lua_State* lua)
	{

	}

	int XMLContent::OnWriteBody(std::ostream& os)
	{
		os.write(this->mXml.c_str(), this->mXml.size());
		return 0;
	}
}

namespace http
{
	void TextContent::OnWriteHead(std::ostream& os)
	{
		os << http::Header::ContentType << ": " << this->mConType << "\r\n";
		os << http::Header::ContentLength << ": " << this->mContent.size() << "\r\n";
	}

	void TextContent::Append(const std::string& content)
	{
		this->mContent.append(content);
	}

	int TextContent::OnWriteBody(std::ostream& os)
	{
		if (!this->mContent.empty())
		{
			//std::cout << this->mContent << std::endl;
			os.write(this->mContent.c_str(), this->mContent.size());
			return 0;
		}
		return 0;
	}

	void TextContent::WriteToLua(lua_State* lua)
	{
		lua_pushlstring(lua, this->mContent.c_str(), this->mContent.size());
	}

	int TextContent::OnRecvMessage(std::istream& is, size_t size)
	{
		std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);
		size_t count = is.readsome(buffer.get(), size);
		if (count > 0)
		{
			this->mContent.append(buffer.get(), count);
			if (this->mContent.size() >= this->mMaxSize)
			{
				return tcp::PacketLong;
			}
		}
		return tcp::ReadSomeMessage;
	}

	void TextContent::SetContent(const std::string& type, const std::string& content)
	{
		this->mConType = type;
		this->mContent = content;
	}

	void TextContent::SetContent(const std::string& type, const char* content, size_t size)
	{
		this->mConType = type;
		this->mContent.assign(content, size);
	}
}

namespace http
{
	FileContent::FileContent()
	{
		this->mFileSize = 0;
		this->mSendSize = 0;
		this->mMaxSize = 0;
		this->mType = http::Header::Bin;
	}

	FileContent::FileContent(std::string t)
			: mType(std::move(t))
	{
		this->mFileSize = 0;
		this->mSendSize = 0;
		this->mMaxSize = 0;
	}

	FileContent::~FileContent()
	{
		if (this->mFile.is_open())
		{
			this->mFile.close();
		}
	}

	bool FileContent::OnDecode()
	{
		if (this->mFile.is_open())
		{
			this->mFile.close();
		}
		return true;
	}

	void FileContent::WriteToLua(lua_State* l)
	{
		lua_pushlstring(l, this->mPath.c_str(), this->mPath.size());
	}

	void FileContent::OnWriteHead(std::ostream& os)
	{
		os << http::Header::ContentType << ": " << this->mType << "\r\n";
		os << http::Header::ContentLength << ": " << this->mFileSize << "\r\n";
	}

	int FileContent::OnRecvMessage(std::istream& is, size_t size)
	{
		std::unique_ptr<char[]> buff(new char[size]);
		size_t count = is.readsome(buff.get(), size);
		if (count > 0)
		{
			this->mFileSize += count;
			this->mFile.write(buff.get(), count);
			this->mFile.flush();
		}
		return tcp::ReadSomeMessage;
	}

	bool FileContent::MakeFile(const std::string& path)
	{
		std::string director;
		if (!help::dir::GetDirByPath(path, director))
		{
			return false;
		}
		if (!help::dir::DirectorIsExist(director))
		{
			help::dir::MakeDir(director);
		}
		this->mFile.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
		if (!this->mFile.is_open())
		{
			return false;
		}
		this->mPath = path;
		return true;
	}

	bool FileContent::OpenFile(const std::string& path)
	{
		this->mFile.open(path, std::ios::in | std::ios::binary);
		if (!this->mFile.is_open())
		{
			return false;
		}
		this->mPath = path;
		help::fs::GetFileSize(this->mFile, this->mFileSize);
		return true;
	}

	bool FileContent::OpenFile(const std::string& path, const std::string& type)
	{
		this->mType = type;
		this->mPath = path;
		return this->OpenFile(path);
	}

	int FileContent::OnWriteBody(std::ostream& os)
	{
		thread_local static char buff[1024] = { 0 };
		size_t size = this->mFile.read(buff, sizeof(buff)).gcount();
		if (size > 0)
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
	bool TransferContent::OpenFile(const std::string& path, const std::string& t)
	{
		this->mType = t;
		this->mPath = path;
		this->mFile.open(path, std::ios::in);
		return this->mFile.is_open();
	}

	void TransferContent::WriteToLua(lua_State* l)
	{
		lua_pushlstring(l, this->mPath.c_str(), this->mPath.size());
	}

	bool TransferContent::OnDecode()
	{
		this->mFile.flush();
		this->mFile.close();
		return true;
	}

	int TransferContent::OnRecvMessage(std::istream& buffer, size_t size)
	{
		if (this->mContSize == 0)
		{
			std::string lineData;
			if (!std::getline(buffer, lineData))
			{
				return tcp::ReadDecodeError;
			}
			if (lineData.empty())
			{
				return tcp::ReadOneLine;
			}
			if (!lineData.empty() && lineData.back() == '\r')
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

	void TransferContent::OnWriteHead(std::ostream& os)
	{
		//os << http::Header::TransferEncoding << ": chunked" << http::CRLF;
		os << http::Header::ContentType << fmt::format(": {}; charset=utf-8\r\n", this->mType);
	}

	int TransferContent::OnWriteBody(std::ostream& os)
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
		os << "0\r\n\r\n";
		this->mFile.close();
		return 0;
	}
}

namespace http
{
	MultipartFromContent::MultipartFromContent()
			: mDone(false), mMaxCount(1024 * 1024 * 5), mReadCount(0)
	{
		this->mLength = 0;
	}

	bool MultipartFromContent::OnDecode()
	{
		this->mDone = true;
		this->mFile.close();
		return true;
	}

	bool MultipartFromContent::Add(const std::string& k, const std::string& v)
	{
		if (!help::fs::FileIsExist(v))
		{
			auto iter = this->mFromData.find(k);
			if (iter != this->mFromData.end())
			{
				return false;
			}
			std::stringstream ss;
			ss << http::Header::ContentDisposition << ": form-data; ";
			ss << fmt::format("name=\"{}\"", k) << "\r\n\r\n" << v << "\r\n";
			this->mHeader.emplace_back(ss.str());
			return true;
		}
		this->mFile.open(v, std::ios::in | std::ios::binary);
		if (!this->mFile.is_open())
		{
			return false;
		}
		this->mLength = 0;
		std::string fileName, fileType;
		if (!help::Str::GetFileName(v, fileName) && help::fs::GetFileType(v, fileType))
		{
			return false;
		}
		this->mPath = v;
		std::stringstream ss;
		std::string contenType(http::GetContentType(fileType));
		ss << http::Header::ContentDisposition << ": form-data; ";
		ss << fmt::format("name=\"{}\"; filename=\"{}\"", k, fileName) << "\r\n";
		ss << http::Header::ContentType << ": " << contenType << "\r\n\r\n";

		this->mHeader.emplace_back(ss.str());
		return true;
	}

	size_t MultipartFromContent::GetContentLength() const
	{
		size_t size = 0;
		size_t length = 0;
		if (help::fs::GetFileSize(this->mPath, size))
		{
			length += size;
		}
		for (size_t i = 0; i < this->mHeader.size(); i++)
		{
			length += this->mHeader[i].size();
			length += (this->mBoundary.size() + 4);
		}
		length += (this->mBoundary.size() + 8);
		return length;
	}

	int MultipartFromContent::OnWriteBody(std::ostream& os)
	{
		for (const std::string& value: this->mHeader)
		{
			os << "--" << this->mBoundary << "\r\n";
			os.write(value.c_str(), value.size());
		}

		char buff[1024] = { 0 };
		while (!this->mFile.eof())
		{
			this->mFile.read(buff, sizeof(buff));
			size_t len = this->mFile.gcount();
			if (len > 0)
			{
				os.write(buff, len);
			}
		}
		os << "\r\n--" << this->mBoundary << "--\r\n";
		/*
				std::stringstream ss;
				ss << os.rdbuf();
				std::string str = ss.str();
				help::fs::WriterFile("./a.json", str);
			*/
		return 0;
	}

	void MultipartFromContent::OnWriteHead(std::ostream& os)
	{
		this->mBoundary = fmt::format("----{}", help::ID::Create());
		os << http::Header::ContentType << ": " << "multipart/form-data; boundary=" << this->mBoundary << "\r\n";
		os << http::Header::ContentLength << ": " << this->GetContentLength() << "\r\n";

	}

	void MultipartFromContent::WriteToLua(lua_State* l)
	{
		lua_pushlstring(l, this->mPath.c_str(), this->mPath.size());
	}

	void MultipartFromContent::Init(const std::string& dir, size_t limit)
	{
		this->mDir = dir;
		this->mReadCount = 0;
		this->mMaxCount = limit;
	}

	std::string MultipartFromContent::ToStr() const
	{
		json::w::Document document;
		document.Add("path", this->mPath);
		document.Add("name", this->mFileName);
		document.Add("size", this->mReadCount);
		document.Add("type", this->mContType);
		return document.JsonString();
	}

	int MultipartFromContent::OnRecvMessage(std::istream& buffer, size_t size)
	{
		this->mReadCount += size;
		if (this->mMaxCount > 0 && this->mReadCount >= this->mMaxCount)
		{
			return tcp::ReadDecodeError;
		}
		if (this->mBoundary.empty())
		{
			if (!std::getline(buffer, this->mBoundary))
			{
				return tcp::ReadOneLine;
			}
			this->mReadCount += this->mBoundary.size() + 1;
			if (this->mBoundary.back() == '\r')
			{
				this->mBoundary.pop_back();
			}
			return tcp::ReadOneLine;
		}

		if (this->mFile.is_open())
		{
			std::unique_ptr<char[]> buff = std::make_unique<char[]>(size);
			size_t count = buffer.readsome(buff.get(), size);
			if (count > 0)
			{
				if (strstr(buff.get(), this->mBoundary.c_str()) != NULL)
				{
					this->mFile.close();
					return tcp::ReadOneLine;
				}
				this->mFile.write(buff.get(), count);
				this->mFile.flush();
			}
			return tcp::ReadOneLine;
		}

		std::string line;
		if (std::getline(buffer, line))
		{
			if (!line.empty() && line.back() == '\r')
			{
				line.pop_back();
			}

			if (line == this->mBoundary)
			{
				return tcp::ReadOneLine;
			}
			else if (line.empty())
			{
				return tcp::ReadOneLine;
			}
			else if (line.find("Content-Disposition:") != std::string::npos)
			{
				size_t name_pos = line.find("name=\"");
				if (name_pos != std::string::npos)
				{
					size_t name_end = line.find("\"", name_pos + 6);
					this->mFieldName = line.substr(name_pos + 6, name_end - name_pos - 6);
				}

				size_t filename_pos = line.find("filename=\"");
				if (filename_pos != std::string::npos)
				{
					size_t filename_end = line.find("\"", filename_pos + 10);
					this->mFileName = line.substr(filename_pos + 10, filename_end - filename_pos - 10);
				}
				return tcp::ReadOneLine;
			}

			if (!this->mFileName.empty())
			{
				std::string director;
				this->mFromData.emplace(this->mFieldName, this->mFileName);
				this->mPath = fmt::format("{}/{}", this->mDir, this->mFileName);

				if (help::dir::GetDirByPath(this->mPath, director))
				{
					help::dir::MakeDir(director);
				}
				this->mFile.open(this->mPath, std::ios::out | std::ios::binary);
				if (!this->mFile.is_open())
				{
					return tcp::ReadDecodeError;
				}
				std::getline(buffer, line);
				//this->mFileName.clear();
			}
			else if (!this->mFieldName.empty())
			{
				if (!line.empty() && line.back() == '\r')
				{
					line.pop_back();
				}
				if (!line.empty())
				{
					this->mFromData.emplace(this->mFieldName, line);
				}
				//std::cout << this->mFieldName << ": " << line << std::endl;
			}
		}
		return tcp::ReadOneLine;
	}

	BinContent::BinContent()
	{

	}

	void BinContent::OnWriteHead(std::ostream& os)
	{
		os << http::Header::ContentType << ": " << http::Header::PB << "\r\n";
		os << http::Header::ContentLength << ": " << this->mBody.size() << "\r\n";
	}

	int BinContent::OnWriteBody(std::ostream& os)
	{
		os.write(this->mBody.c_str(), this->mBody.size());
		return 0;
	}

	void BinContent::WriteToLua(lua_State* l)
	{
		lua_pushlstring(l, this->mBody.c_str(), this->mBody.size());
	}

	int BinContent::OnRecvMessage(std::istream& is, size_t size)
	{
		std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);
		size_t count = is.readsome(buffer.get(), size);
		if (count > 0)
		{
			this->mBody.append(buffer.get(), count);
		}
		return tcp::ReadSomeMessage;
	}

}