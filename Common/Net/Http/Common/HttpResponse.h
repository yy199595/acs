//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPRESPONSE_H
#define APP_HTTPRESPONSE_H
#include<fstream>
#include <memory>
#include"httpHead.h"
#include"Http/Common/Content.h"
#include"Lua/Engine/Define.h"
#include"Proto/Message/IProto.h"
#include"Yyjson/Document/Document.h"


namespace http
{
	class Response : public tcp::IProto, public ILuaWrite
    {
	 public:
		Response();
		~Response() override = default;
	public:
		void Clear() override;
		Head & Header() { return this->mHead; }
		HttpStatus Code() const { return (HttpStatus)this->mCode; }
		const std::string & GetError() const { return this->mError; }
	public:
		template<typename T>
		inline const T * To() const;
		inline const Content * GetBody() const { return this->mBody.get(); }
	public:
		void SetCode(HttpStatus code);
		void Json(const std::string & json);
		void Json(const char * json, size_t);
		void Text(const char * text, size_t);
		void Json(json::w::Document & document);
		bool OpenOrCreateFile(const std::string & path);
		bool File(const std::string & type, const std::string & path);
		void SetContent(const std::string & type, const std::string& str);
		inline void SetContent(std::unique_ptr<Content> data) { this->mBody = std::move(data); }
	public:
		std::string ToString() final;
		int WriteToLua(lua_State *lua) const final;
		int OnSendMessage(std::ostream &os) final;
		int OnRecvMessage(std::istream &os, size_t size) final;
	private:
        int mCode;
		Head mHead;
		int mParseState;
		long long mContSize;
		std::string mError;
		std::string mChunked;
		std::string mVersion;
		std::unique_ptr<http::Content> mBody;
	};

	template<typename T> inline const T* Response::To() const
	{
		if(this->mBody == nullptr)
		{
			return nullptr;
		}
		return dynamic_cast<T*>(this->mBody.get());
	}
}


#endif //APP_HTTPRESPONSE_H
