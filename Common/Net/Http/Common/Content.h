//
// Created by yy on 2023/11/19.
//

#ifndef APP_CONTENT_H
#define APP_CONTENT_H
#include<iostream>
#include<fstream>
#include<queue>
#include<cstdio>
#include<unordered_map>
#include <utility>
#include "IContent.h"
#include"Proto/Message/IProto.h"
#include "Core/Map/HashMap.h"
#include"XML/Document/XDocument.h"
#include<Yyjson/Document/Document.h>

namespace http
{
    class FromContent : public Content
    {
    public:
        inline void Clear() { this->mParameters.clear(); }
        auto End() const { return this->mParameters.end(); }
        auto Begin() const { return this->mParameters.begin(); }
        size_t Size() const { return this->mParameters.size(); }
    public:
		std::string ToStr() const final;
		void WriteToLua(lua_State *l) final;
		bool Add(const std::string & k, int v);
        bool Add(const std::string & k, const std::string & v);
	public:
		void Set(const std::string & k, int v);
		void Set(const std::string & k, const std::string & v);
    public:
		std::string Serialize() const;
		bool Decode(const std::string & content);
        bool Get(std::vector<std::string> & keys) const;
        bool Get(const std::string & k, int & value) const;
		bool Get(const std::string & k, long long & value) const;
		bool Get(const std::string & key, std::string & value) const;
        http::ContentType GetContentType() const final { return http::ContentType::FROM; }
    private:
        bool OnDecode() final;
        void OnWriteHead(std::ostream &os) final;
        int OnWriteBody(std::ostream &os) final { return 0; }
        int OnRecvMessage(std::istream & ss, size_t size) final;
		int ContentLength() final { return (int)this->mContent.size();}
    private:
        std::string mContent;
        std::unordered_map<std::string, std::string> mParameters;
    };
}

namespace http
{
    class JsonContent : public Content, public json::r::Document
    {
	public:
		explicit JsonContent() = default;
		explicit JsonContent(std::string  json) : mJson(std::move(json)) { }
		explicit JsonContent(const char * json, size_t size) : mJson(json, size) { }
		~JsonContent() override = default;
    public:
		void Write(const json::w::Document & document);
		const std::string & JsonStr() const { return this->mJson;}
		http::ContentType GetContentType() const final { return http::ContentType::JSON; }
    private:
		void WriteToLua(lua_State *l) final;
        void OnWriteHead(std::ostream &os) final;
        int OnWriteBody(std::ostream &os) final;
        int OnRecvMessage(std::istream & is, size_t size) final;
		bool OnDecode() final { return this->Decode(this->mJson); }
		std::string ToStr() const final { return this->mJson; }
		int ContentLength() final { return (int)this->mJson.size(); }
	private:
        std::string mJson;
	};
}

namespace http
{
	class XMLContent : public Content, public xml::XDocument
	{
	public:
		explicit XMLContent() = default;
		explicit XMLContent(std::string  xml) : mXml(std::move(xml)) { }
		explicit XMLContent(const char* xml, size_t size) : mXml(xml, size) { }
		~XMLContent() override = default;
	public:
		http::ContentType GetContentType() const final { return http::ContentType::XML; }
	private:
		void WriteToLua(lua_State* l) final;
		void OnWriteHead(std::ostream& os) final;
		int OnWriteBody(std::ostream& os) final;
		int OnRecvMessage(std::istream& is, size_t size) final;
		bool OnDecode() final { return this->Decode(this->mXml); }
		std::string ToStr() const final { return this->mXml; }
		int ContentLength() final { return (int)this->mXml.size(); }
	private:
		std::string mXml;
	};
}

namespace http
{
	class TextContent : public Content
	{
	public:
		explicit TextContent() : mMaxSize(1024 * 500) { }
		explicit TextContent(std::string t) : mMaxSize(10240), mConType(std::move(t)) {}
	private:
		void WriteToLua(lua_State *l) override;
		bool OnDecode() final { return true; }
		int OnWriteBody(std::ostream &os) final;
		void OnWriteHead(std::ostream &os) final;
		int OnRecvMessage(std::istream & is, size_t size) final;
		std::string ToStr() const final { return this->mContent; }
		int ContentLength() final { return (int)this->mContent.size();}
	public:
		void Append(const std::string & content);
		std::string * Data() { return &this->mContent;}
		const std::string &Content() const { return this->mContent; }
		inline void SetMaxSize(size_t size) { this->mMaxSize = size; }
		void SetContent(const std::string & type, const std::string & content);
		inline void SetContentType(const std::string & t) { this->mConType = t; }
		void SetContent(const std::string & type, const char * content, size_t size);
		http::ContentType GetContentType() const final { return http::ContentType::TEXT; }
	private:
		size_t mMaxSize;
		std::string mUrl;
		std::string mConType;
		std::string mContent;
	};
}

namespace http
{
	class FileContent : public Content
	{
	public:
		FileContent();
		explicit FileContent(std::string t);
		~FileContent() override;
	public:
		bool OpenFile(const std::string & path);
		bool MakeFile(const std::string & path);
		inline void SetMaxSize(size_t size) { this->mMaxSize = size; }
		bool OpenFile(const std::string & path, const std::string & type);
		http::ContentType GetContentType() const final { return http::ContentType::FILE; }
	private:
		bool OnDecode() final;
		void WriteToLua(lua_State *l) final;
		int OnWriteBody(std::ostream &os) final;
		void OnWriteHead(std::ostream &os) final;
		std::string ToStr() const final { return this->mPath; }
		int ContentLength() final { return (int)this->mFileSize;}
		int OnRecvMessage(std::istream & is, size_t) final;
	public:
		const std::string & Path() const { return this->mPath; }
	private:
		size_t mMaxSize;
		size_t mFileSize;
		size_t mSendSize;
		std::string mPath;
		std::string mType;
		std::fstream mFile;
	};
}

namespace http
{
	class TransferContent : public Content
	{
	public:
		TransferContent() : mContSize(0) { }
		~TransferContent() override { this->mFile.close(); }
	public:
		bool OpenFile(const std::string & path, const std::string & t);
	private:
		bool OnDecode() final;
		void WriteToLua(lua_State *l) final;
		int OnWriteBody(std::ostream &os) final;
		void OnWriteHead(std::ostream &os) final;
		int OnRecvMessage(std::istream & is, size_t size) final;
		std::string ToStr() const final { return this->mPath; }
		int ContentLength() final { return 0; }
		http::ContentType GetContentType() const final { return http::ContentType::CHUNKED; }
	private:
		size_t mContSize;
		std::string mPath;
		std::string mType;
		std::fstream mFile;
	};
}

namespace http
{
	class MultipartFromContent : public Content
	{
	public:
		MultipartFromContent();
		~MultipartFromContent() final { this->mFile.close(); }
	public:
		bool OnDecode() final;
		void WriteToLua(lua_State *l) final;
		int OnRecvMessage(std::istream & buffer, size_t size) final;
		void Init(const std::string & dir, size_t limit = 1024 * 1024 * 2); //最大2M
	public:
		inline bool IsDone() const { return this->mDone; }
		inline const std::string & Path() const { return this->mPath; }
		inline const std::string & FileName() const { return this->mFileName; }
		inline const std::string & ContentType() const { return this->mContType; }
	public:
		bool Add(const std::string& k, const std::string& v);
	private:
		size_t GetContentLength() const;
		std::string ToStr() const final;
		int OnWriteBody(std::ostream& os) final;
		void OnWriteHead(std::ostream& os) final;
		int ContentLength() final { return (int)this->mReadCount;}
		http::ContentType GetContentType() const final { return http::ContentType::MULTIPAR; }
	private:
		bool mDone;
		size_t mLength;
		size_t mMaxCount;
		std::string mDir;
		std::string mPath;
		size_t mReadCount;
		std::fstream mFile;
		std::string mFileName;
		std::string mFieldName;
		std::string mContType;
		std::string mBoundary;
		std::string mContent;
		std::vector<std::string> mHeader;
		std::unordered_map<std::string, std::string> mFromData;
	};
}

namespace http
{
	class BinContent : public Content
	{
	public:
		BinContent();
	private:
		void WriteToLua(lua_State *l) final;
		bool OnDecode() final { return true; }
		int OnWriteBody(std::ostream &os) final;
		void OnWriteHead(std::ostream &os) final;
		int OnRecvMessage(std::istream &is, size_t size) final;
		int ContentLength() final { return (int)this->mBody.size();}
		http::ContentType GetContentType() const final { return http::ContentType::PB; }
	public:
		const std::string & GetContent() const { return this->mBody; }
	private:
		std::string mBody;
	};
}

#endif //APP_CONTENT_H
