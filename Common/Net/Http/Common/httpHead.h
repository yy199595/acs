//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPHEAD_H
#define APP_HTTPHEAD_H
#include<string>
#include<istream>
#include<ostream>
#include<vector>
#include<unordered_map>
#include"Http/Client/Http.h"
#include"Lua/Engine/Define.h"
#include"Proto/Message/IProto.h"
namespace http
{
    enum class DecodeState
    {
        None,
        Head,
        Body
    };
}

namespace http
{
	class Head : public tcp::IProto, public tcp::IHeader
    {
    public:
        Head();
    public:
		std::string ToString() final;
		int OnSendMessage(std::ostream &os) final;
	public:
		void Clear() final;
		bool KeepAlive() const;
		bool GetContentType(std::string & type) const;
		bool GetContentLength(long long & length) const;
		int OnRecvMessage(std::istream &os, size_t size) final;
		inline void SetKeepAlive(bool keep) { this->mKeepAlive = keep; }
    private:
		int mCounter;
		bool mKeepAlive;
    };
}

#endif //APP_HTTPHEAD_H
