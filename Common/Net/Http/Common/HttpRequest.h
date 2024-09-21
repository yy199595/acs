//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPREQUEST_H
#define APP_HTTPREQUEST_H
#include<string>
#include<fstream>
#include<vector>
#include<memory>
#include"httpHead.h"
#include<unordered_map>
#include"Http/Common/Url.h"
#include"Http/Common/Content.h"
#include"Proto/Message/IProto.h"

struct lua_State;
namespace http
{
	class Request : public tcp::IProto, public ILuaWrite
    {
    public:
		explicit Request();
		explicit Request(const char * method);
	public:
		std::string ToString() final;
        inline Head & Header() { return this->mHead; }
	public:
		bool GetIp(std::string & ip) const;
		inline int Timeout() const { return this->mTimeout;}
		inline void SetSockId(int id) { this->mSockId = id; }
		inline int GetSockId() const { return this->mSockId; }
		inline bool IsHttps() const { return this->mUrl.IsHttps(); }
		inline const http::Url & GetUrl() const { return this->mUrl; }
	public:
		inline const Content * GetBody() const { return this->mBody.get();}
		inline const http::Head & ConstHeader() const { return this->mHead; }
		inline void SetBody(std::unique_ptr<Content> body) { this->mBody = std::move(body);}
	public:
		bool SetUrl(const std::string & url);
		bool SetUrl(const std::string & url, const http::FromContent & query);
		int WriteToLua(lua_State *lua) const final;
		inline void SetTimeout(int second) { this->mTimeout = second; }
	public:
		const std::string & GetVerifyFile() const { return this->mVerifyFile; }
		void SetVerifyFile(const std::string & path) { this->mVerifyFile = path; }
	public:
		bool IsMethod(const std::string & method) const;
		void SetContent(const json::w::Document & document);
		void SetContent(const char * t, const std::string & content);
		void SetContent(const char * t, const char * content, size_t size);
	public:
		void Clear() final;
        int OnSendMessage(std::ostream &os) final;
		int OnRecvMessage(std::istream &os, size_t size) final;
	private:
		void WriteMessageToLua(lua_State * lua) const;
	private:
        Head mHead;
		int mSockId;
		int mTimeout;
		http::Url mUrl;
		int mDecodeStatus;
		long long mConeSize;
		std::string mVerifyFile;
		std::unique_ptr<http::Content> mBody;
    };
}


#endif //APP_HTTPREQUEST_H
