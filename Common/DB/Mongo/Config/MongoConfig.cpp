#include "MongoConfig.h"
#include <regex>
#include <spdlog/fmt/fmt.h>
namespace mongo
{
	bool MongoConfig::FromString(const std::string& url)
	{
		std::regex re(R"(^(mongodb(?:\+srv)?://)(?:([^:]+)(?::([^@]+))?@)?([^:/]+)(?::(\d+))?(?:/([^?]+))?)");
		std::smatch matches;
		if (!std::regex_match(url, matches, re))
		{
			return false;
		}
		this->DB = matches[6];
		this->User = matches[2];
		this->Password = matches[3];
		const std::string ip = matches[4];
		const std::string port = matches[5];
		this->Address = fmt::format("{}:{}", ip, port);
		if (this->User.empty())
		{
			this->Password.clear();
		}
		return true;
	}
}