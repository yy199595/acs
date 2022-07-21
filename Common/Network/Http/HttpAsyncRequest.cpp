//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpAsyncRequest.h"
#include<regex>
#include"Http.h"
#include<iostream>
#include"Util/StringHelper.h"

namespace Sentry
{
    std::shared_ptr<HttpTask> HttpAsyncRequest::MakeTask(int timeout)
    {
        std::shared_ptr<HttpTask> httpTask = std::make_shared<HttpTask>(timeout);
        this->mTaskId = httpTask->GetRpcId();
        return httpTask;
    }

    std::shared_ptr<LuaHttpTask> HttpAsyncRequest::MakeLuaTask(lua_State *lua, int timeout)
    {
        std::shared_ptr<LuaHttpTask> luaHttpTask = std::make_shared<LuaHttpTask>(lua, timeout);
        this->mTaskId = luaHttpTask->GetRpcId();
        return luaHttpTask;
    }

    LuaHttpTask::LuaHttpTask(lua_State *lua, int timeout)
        : mRef(0)
    {
        this->mLua = lua;
        lua_pushthread(lua);
        this->mTimeout = timeout;
        this->mTaskId = Helper::Guid::Create();
        if(lua_isthread(this->mLua, -1))
        {
            this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
        }
    }

    LuaHttpTask::~LuaHttpTask()
    {
        if(this->mRef == 0)
        {
            luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        }
    }

    void LuaHttpTask::OnResponse(std::shared_ptr<HttpAsyncResponse> response)
    {
        lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        lua_State* coroutine = lua_tothread(this->mLua, -1);
        if(response != nullptr)
        {
            response->GetData().Writer(this->mLua);
            lua_presume(coroutine, this->mLua, 1);
            return;
        }
        lua_presume(coroutine, this->mLua, 0);
    }

    int LuaHttpTask::Await(std::shared_ptr<HttpRequestClient> client)
    {
        if(this->mRef == 0)
        {
            luaL_error(this->mLua, "not lua coroutine context yield failure");
            return 0;
        }
		this->mRequestClient = client;
        return lua_yield(this->mLua, 0);
    }
}

namespace Sentry
{
	HttpAsyncRequest::HttpAsyncRequest(const std::string& method)
		: mMethod(method)
	{
        this->mTaskId = 0;
	}

    bool HttpAsyncRequest::ParseUrl(const std::string &url)
    {
        std::cmatch what;
        std::string protocol;
        std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");
        if (std::regex_match(url.c_str(), what, pattern))
		{
            this->mHost = std::string(what[2].first, what[2].second);
            this->mPath = std::string(what[4].first, what[4].second);
            protocol = std::string(what[1].first, what[1].second);
            this->mPort = std::string(what[3].first, what[3].second);

            if (0 == this->mPort.length()) {
                this->mPort = "http" == protocol ? "80" : "443";
            }
            return true;
        }
        return false;
    }

	bool HttpAsyncRequest::AddHead(const std::string& key, int value)
	{
		auto iter = this->mHeadMap.find(key);
		if(iter != this->mHeadMap.end())
		{
			return false;
		}
		this->mHeadMap.emplace(key, std::to_string(value));
		return true;
	}

	bool HttpAsyncRequest::AddHead(const std::string& key, const std::string& value)
	{
		auto iter = this->mHeadMap.find(key);
		if(iter != this->mHeadMap.end())
		{
			return false;
		}
		this->mHeadMap.emplace(key, value);
		return true;
	}
    int HttpAsyncRequest::Serailize(std::ostream& os)
	{
		os << this->mMethod << " " << this->mPath << " " << HttpVersion << "\r\n";
		os << "Host:" << this->mHost << "\r\n";
		for(auto iter = this->mHeadMap.begin(); iter != this->mHeadMap.end(); iter++)
		{
			const std::string & key = iter->first;
			const std::string & value = iter->second;
			os << key << ':' << value << "\r\n";
		}
		os << "Connection: close\r\n\r\n";
		this->WriteBody(os);
		return 0;
	}
}

namespace Sentry
{
    void HttpData::Add(const std::string &data)
    {
        this->mData.append(data);
    }

    void HttpData::Add(const char *data, size_t size)
    {
        this->mData.append(data, size);
    }

    void HttpData::Add(const std::string &key, const std::string &data)
    {
        this->mHead.emplace(key, data);
    }

    bool HttpData::Get(const std::string &key, std::string &value) const
    {
        auto iter = this->mHead.find(key);
        if(iter != this->mHead.end())
        {
            value = iter->second;
            return true;
        }
        return false;
    }

    void HttpData::WriterString(lua_State *lua, const char *name, const std::string &str) const
    {
        if(!str.empty() && name != nullptr)
        {
            lua_pushlstring(lua, str.c_str(), str.size());
            lua_setfield(lua, -2, name);
        }
    }

    void HttpData::Writer(lua_State *lua) const
    {
        lua_createtable(lua, 0, 6);
        this->WriterString(lua, "data", this->mData);
        this->WriterString(lua, "path", this->mPath);
        this->WriterString(lua, "method", this->mMethod);
        this->WriterString(lua, "version", this->mVersion);
        this->WriterString(lua, "address", this->mAddress);

        lua_createtable(lua, 0, this->mHead.size());
        auto iter = this->mHead.begin();
        for (; iter != this->mHead.end(); iter++)
        {
            const std::string &key = iter->first;
            const std::string &value = iter->second;
            this->WriterString(lua, key.c_str(), value);
        }
        lua_setfield(lua, -2, "head");
    }
}


namespace Sentry
{
	HttpGetRequest::HttpGetRequest()
		: HttpAsyncRequest("GET")
	{

	}

	void HttpGetRequest::WriteBody(std::ostream& os) const
	{

	}

	std::shared_ptr<HttpGetRequest> HttpGetRequest::Create(const std::string& url)
	{
		std::shared_ptr<HttpGetRequest> request(new HttpGetRequest());
		return request->ParseUrl(url) ? request : nullptr;
	}
}

namespace Sentry
{
	HttpTask::HttpTask(int timeout)
	{
		this->mTimeout = timeout;
		this->mTaskId = Helper::Guid::Create();
	}
	void HttpTask::OnResponse(std::shared_ptr<HttpAsyncResponse> response)
	{
		this->mTask.SetResult(response);
	}
}

namespace Sentry
{
	HttpPostRequest::HttpPostRequest()
		: HttpAsyncRequest("POST")
	{

	}

	void HttpPostRequest::AddBody(const std::string& content)
	{
		this->mBody = content;
		this->AddHead("content-length", (int)content.size());
	}

	void HttpPostRequest::WriteBody(std::ostream& os) const
	{
		os.write(this->mBody.c_str(), this->mBody.size());
	}

	std::shared_ptr<HttpPostRequest> HttpPostRequest::Create(const std::string& url)
	{
		std::shared_ptr<HttpPostRequest> request(new HttpPostRequest());
		return request->ParseUrl(url) ? request : nullptr;
	}

	void HttpPostRequest::AddBody(const char* data, size_t size)
	{
		this->mBody.append(data, size);
		this->AddHead("content-length", size);
	}
}

namespace Sentry
{
	HttpAsyncResponse::HttpAsyncResponse()
	{
		this->mHttpCode = 0;
		this->mState = HttpDecodeState::FirstLine;
	}

	int HttpAsyncResponse::OnReceiveLine(asio::streambuf& streamBuffer)
	{
		std::iostream io(&streamBuffer);
		if (this->mState == HttpDecodeState::FirstLine)
		{
			this->mState = HttpDecodeState::HeadLine;
			io >> this->mHttpData.mVersion >> this->mHttpCode >> this->mHttpError;
			io.ignore(2); //放弃\r\n
		}
		if (this->mState == HttpDecodeState::HeadLine)
		{
			std::string lineData;
			while (std::getline(io, lineData))
			{
				if (lineData == "\r")
				{
					return 1; //再读
				}
				size_t pos = lineData.find(':');
				if (pos != std::string::npos)
				{
					size_t length = lineData.size() - pos - 2;
					std::string key = lineData.substr(0, pos);
					Helper::String::Tolower(key);
					this->mHttpData.Add(key, lineData.substr(pos + 1, length));
				}
			}
		}
		return -1; //再读一行
	}
}

namespace Sentry
{
	HttpFileResponse::HttpFileResponse(std::fstream* fs)
	{
		this->mFstream = fs;
	}

	HttpFileResponse::~HttpFileResponse() noexcept
	{
		this->mFstream->close();
		delete this->mFstream;
	}

	int HttpFileResponse::OnReceiveSome(asio::streambuf& streamBuffer)
	{
		std::iostream oss(&streamBuffer);
		size_t size = oss.readsome(this->mBuffer, 128);
		while(size > 0)
		{
			this->mFstream->write(this->mBuffer, size);
			size = oss.readsome(this->mBuffer, 128);
		}
		return 1;
	}

	int HttpDataResponse::OnReceiveSome(asio::streambuf& streamBuffer)
	{
		char buffer[128] = {0};
		std::iostream oss(&streamBuffer);
		size_t size = oss.readsome(buffer, 128);
		while(size > 0)
		{
			this->mHttpData.mData.append(buffer, size);
			size = oss.readsome(buffer, 128);
		}
		return 1;
	}


    void HttpAsyncResponse::SetError(const asio::error_code &code)
    {
        this->mCode = code;
        this->mHttpError = code.message();
    }

	bool HttpAsyncResponse::GetHead(const std::string& key, int& value) const
	{
		auto iter = this->mHttpData.mHead.find(key);
		if(iter != this->mHttpData.mHead.end())
		{
			value = std::stoi(iter->second);
			return true;
		}
		return false;
	}

	bool HttpAsyncResponse::GetHead(const std::string& key, std::string& value) const
	{
		auto iter = this->mHttpData.mHead.find(key);
		if(iter != this->mHttpData.mHead.end())
		{
			value = iter->second;
			return true;
		}
		return false;
	}
}

namespace Sentry
{
    HttpHandlerRequest::HttpHandlerRequest(const std::string & address)
    {
		this->mHttpData.mAddress = address;
        this->mState = HttpDecodeState::FirstLine;
    }

    bool HttpHandlerRequest::GetHead(const std::string &key, int &value) const
    {
		auto iter = this->mHttpData.mHead.find(key);
		if(iter != this->mHttpData.mHead.end())
		{
			value = std::stoi(iter->second);
			return true;
		}
		return false;
    }

    bool HttpHandlerRequest::GetHead(const std::string &key, std::string &value) const
    {
		auto iter = this->mHttpData.mHead.find(key);
		if(iter != this->mHttpData.mHead.end())
		{
			value = iter->second;
			return true;
		}
		return false;
    }

    int HttpHandlerRequest::OnReceiveSome(asio::streambuf &streamBuffer)
    {
        if (this->mState != HttpDecodeState::Content)
        {
            return this->OnReceiveLine(streamBuffer);
        }
        char buffer[128] = {0 };
        std::iostream io(&streamBuffer);
        size_t size = io.readsome(buffer, 128);
        while(size > 0)
        {
            this->mHttpData.Add(buffer, size);
            size = io.readsome(buffer, size);
        }
        return 1;
    }

    int HttpHandlerRequest::OnReceiveLine(asio::streambuf &streamBuffer)
    {
        std::iostream io(&streamBuffer);
        if(this->mState == HttpDecodeState::FirstLine)
        {
            this->mState = HttpDecodeState::HeadLine;
            io >> this->mHttpData.mMethod >> this->mUrl >> this->mHttpData.mVersion;

			io.ignore(2); //去掉\r\n
        }
        if(this->mState == HttpDecodeState::HeadLine)
		{
			std::string lineData;
			while (std::getline(io, lineData))
			{
				if (lineData == "\r")
				{
					this->mState = HttpDecodeState::Content;
					break;
				}
				size_t pos = lineData.find(":");
				if (pos != std::string::npos)
				{
					size_t length = lineData.size() - pos - 2;
					std::string key = lineData.substr(0, pos);

					Helper::String::Tolower(key);
                    this->mHttpData.Add(key, lineData.substr(pos + 1, length));
				}
			}
			if (this->mState == HttpDecodeState::Content)
			{
				if (this->GetMethod() == "GET")
				{
					size_t pos = this->mUrl.find("?");
					if (pos != std::string::npos)
					{
                        this->mHttpData.mData = this->mUrl.substr(pos + 1);
                        this->mHttpData.mPath = this->mUrl.substr(0, pos);
					}
					return 0;
				}
				else if (this->GetMethod() != "POST")
				{
					return -2;
				}
				this->mHttpData.mPath = this->mUrl;
                return 1; //读一部分
			}
		}
        return -1; //读一行
    }
}

namespace Sentry
{
	HttpHandlerResponse::HttpHandlerResponse()
	{
		this->mCount = 0;
		this->mContentSize = 0;
		this->mFstream = nullptr;
		this->mCode = HttpStatus::OK;
	}

	HttpHandlerResponse::~HttpHandlerResponse()
	{
		if(this->mFstream != nullptr)
		{
			this->mFstream->close();
			delete this->mFstream;
		}
	}

	int HttpHandlerResponse::Serailize(std::ostream& os)
	{
		if(this->mCount == 0)
		{
			if(this->mFstream == nullptr)
			{
				this->mContentSize = this->mContent.size();
			}
			this->AddHead("content-length", std::to_string(this->mContentSize));
			os << HttpVersion << ' ' << (int)this->mCode << ' ' << HttpStatusToString(this->mCode) << "\r\n";
			for (auto iter = this->mHeadMap.begin(); iter != this->mHeadMap.end(); iter++)
			{
				const std::string& key = iter->first;
				const std::string& value = iter->second;
				os << key << ":" << value << "\r\n";
			}
			os << "\r\n";
		}
		this->mCount++;
		if(this->mFstream != nullptr)
		{
			char buffer[512] = { 0 };
			this->mFstream->read(buffer, 512);
			if(this->mFstream->gcount() > 0)
			{
				os.write(buffer, this->mFstream->gcount());
				return this->mContentSize - this->mFstream->gcount();
			}
			return 0;
		}
		os.write(this->mContent.c_str(), this->mContent.size());
		return 0;
	}

    bool HttpHandlerResponse::AddHead(const char *key, int value)
    {
        return this->AddHead(key, std::to_string(value));
    }

	bool HttpHandlerResponse::AddHead(const char * key, const std::string& value)
	{
		auto iter = this->mHeadMap.find(key);
		if(iter != this->mHeadMap.end())
		{
			return false;
		}
		this->mHeadMap.emplace(key, value);
		return true;
	}

	void HttpHandlerResponse::SetCode(HttpStatus code)
	{
		this->mCode = code;
	}

	void HttpHandlerResponse::WriteString(const std::string& content)
    {
        this->mContentSize += content.size();
        this->mContent.append(content);
    }

    void HttpHandlerResponse::WriteString(const char *content, size_t size)
    {
        this->mContentSize += size;
        this->mContent.append(content, size);
    }

	void HttpHandlerResponse::WriteFile(std::fstream * ofstream)
	{
		this->mFstream = ofstream;
		this->mFstream->seekg(0, this->mFstream->end);
		this->mContentSize = ofstream->tellg();
		this->mFstream->seekp(0, this->mFstream->beg);
	}
}