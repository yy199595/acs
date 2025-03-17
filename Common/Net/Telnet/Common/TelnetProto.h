//
// Created by yy on 2025/3/4.
//

#ifndef APP_TELNETPROTO_H
#define APP_TELNETPROTO_H
#include <utility>
#include <vector>
#include "Proto/Message/IProto.h"
namespace telnet
{
	class Request : public tcp::IProto
	{
	public:
		Request() = default;
	public:
		inline void Clear() final { this->mCmd.clear(); }
		inline int OnRecvMessage(std::istream &os, size_t size) final;
		inline int OnSendMessage(std::ostream &os) final { return 0;}
	public:
		inline const std::string & GetCmd() const { return this->mCmd; }
		inline const std::vector<std::string> & GetArgs() const { return this->mArgs; }
	private:
		std::string mCmd;
		std::vector<std::string> mArgs;
	};

	int Request::OnRecvMessage(std::istream& os, size_t size)
	{
		std::string message;
		for(size_t index = 0; index < size; index++)
		{
			char cc = (char)os.get();
			if (cc == '\b')
			{
				if (!message.empty())
				{
					message.pop_back();
				}
			}
			else
			{
				message += cc;
			}
		}
		if(message.size() >= 2)
		{
			message.pop_back();
			message.pop_back();
		}
		std::stringstream ss(message);
		if(this->mCmd.empty())
		{
			if(!std::getline(ss, this->mCmd, ' '))
			{
				this->mCmd = message;
				return 0;
			}
		}
		std::string args;
		while(std::getline(ss, args, ' '))
		{
			this->mArgs.emplace_back(args);
			args.clear();
		}
		return 0;
	}
}

namespace telnet
{
	class Response : public tcp::IProto
	{
	public:
		Response() = default;
		explicit Response(std::string message) : mMessage(std::move(message)) { }
	public:
		inline int OnSendMessage(std::ostream &os) final;
		inline void Clear() final { this->mMessage.clear(); }
		inline void Append(const std::string & msg) { this->mMessage.append(msg); }
		inline int OnRecvMessage(std::istream &os, size_t size) final { return 0;}
	private:
		std::string mMessage;
	};

	inline int Response::OnSendMessage(std::ostream& os)
	{
		os.write(this->mMessage.c_str(), this->mMessage.size());
		os << "\r\n";
		return 0;
	}
}

#endif //APP_TELNETPROTO_H
