//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPHEAD_H
#define APP_HTTPHEAD_H
#include<string>
#include<istream>
#include<ostream>
#include<unordered_map>
#include"Http/Client/Http.h"

#define HTTP_READ_LINE -1
#define HTTP_READ_SOME -2
#define HTTP_READ_ERROR -3
#define HTTP_READ_COMPLETE -0

namespace Http
{
    enum class DecodeState
    {
        None,
        Head,
        Body
    };
    class IStream
    {
    public:
        virtual int OnRead(std::istream & buffer) = 0; //true 解析完成 false 解析失败
        virtual int OnWrite(std::ostream & buffer) = 0;
        virtual void OnComplete() { };
    };
}

namespace Http
{
    class Head : public IStream
    {
    public:
        Head() : mContentLength(0) { }
        bool Add(const std::string & k, int v);
        bool Add(const std::string & k, const std::string & v);
    public:
		bool Get(const std::string& k, int& v) const;
        bool Get(const std::string& k, long long& v) const;
        bool Get(const std::string & k, std::string & v) const;
    public:
        int OnRead(std::istream & buffer) final;
        int OnWrite(std::ostream & buffer) final;
		int ContentLength() const { return this->mContentLength; }
		const std::string & ContentType() const { return this->mContentType;}
    private:
		int mContentLength;
		std::string mContentType;
        std::unordered_map<std::string, std::string> mHeads;
    };
}

#endif //APP_HTTPHEAD_H
