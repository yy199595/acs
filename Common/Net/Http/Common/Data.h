//
// Created by yy on 2023/11/19.
//

#ifndef APP_DATA_H
#define APP_DATA_H
#include<iostream>
#include<fstream>
#include<queue>
#include<cstdio>
#include<unordered_map>
#include"Core/Zip/miniz.h"
#include"Http/Client/Http.h"
#include"Proto/Message/IProto.h"
#include "Core/Map/HashMap.h"
#include<Yyjson/Document/Document.h>

struct lua_State;

namespace http
{
    class Data
    {
	public:
		Data() = default;
		virtual ~Data() = default;
    public:
        virtual bool OnDecode() = 0;
		virtual void WriteToLua(lua_State * l) = 0;
		virtual std::string ToStr() const { return ""; }
        virtual void OnWriteHead(std::ostream & os) = 0;
        virtual int OnWriteBody(std::ostream & os) = 0;
        virtual int OnRecvMessage(std::istream & is, size_t size) = 0;
		virtual int ContentLength() = 0;
	public:
		template<typename T>
		T * Cast() { return static_cast<T*>(this); }
		template<typename T>
		const T * To() const { return dynamic_cast<T*>(this); }
    public:
        virtual http::ContentType GetContentType() const = 0;
    };
}

namespace http
{
    class FromData : public Data
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
    class JsonData : public Data, public json::r::Document
    {
	public:
		explicit JsonData() { }
		explicit JsonData(const std::string & json) :mJson(json) { }
		explicit JsonData(const char * json, size_t size) : mJson(json, size) { }
		~JsonData() override = default;
    public:
		void Write(json::w::Document & document);
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
	class TextData : public Data
	{
	public:
		explicit TextData() : mMaxSize(10240), mDeflate(false) { }
		explicit TextData(std::string t) : mDeflate(false), mMaxSize(10240), mConType(std::move(t)) {}
	private:
		void WriteToLua(lua_State *l) final;
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
		bool mDeflate;
		size_t mMaxSize;
		std::string mConType;
		std::string mContent;
	};
}

namespace http
{
	class FileData : public Data
	{
	public:
		FileData();
		explicit FileData(std::string t);
		~FileData() override;
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
	class TransferData : public Data
	{
	public:
		TransferData() : mContSize(0) { }
		~TransferData() { this->mFile.close(); }
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
	class MultipartFromData : public Data
	{
	public:
		MultipartFromData();
		~MultipartFromData() final { this->mFile.close(); }
	public:
		bool OnDecode() final;
		void WriteToLua(lua_State *l) final;
		int OnRecvMessage(std::istream & buffer, size_t size) final;
		void Init(const std::string & dir, const std::string & name, size_t limit = 1024 * 1024 * 2); //最大2M
	public:
		inline bool IsDone() const { return this->mDone; }
		inline const std::string & Path() const { return this->mPath; }
		inline const std::string & FileName() const { return this->mFileName; }
		inline const std::string & ContentType() const { return this->mContType; }
	private:
		std::string ToStr() const final;
		int OnWriteBody(std::ostream &os) final;
		void OnWriteHead(std::ostream &os) final { }
		int ContentLength() final { return (int)this->mReadCount;}
		http::ContentType GetContentType() const final { return http::ContentType::MULTIPAR; }
	private:
		bool mDone;
		size_t mMaxCount;
		std::string mDir;
		std::string mPath;
		size_t mReadCount;
		std::fstream mFile;
		std::string mFileName;
		std::string mContType;
		std::string mEndBoundy;
		custom::HashMap<std::string, std::string> mHeader;
	};
}

#endif //APP_DATA_H