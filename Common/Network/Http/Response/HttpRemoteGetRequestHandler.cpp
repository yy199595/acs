//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpRemoteGetRequestHandler.h"
#include <Util/StringHelper.h>
#include <Define/CommonDef.h>
namespace GameKeeper
{

	bool HttpRemoteGetRequestHandler::SplitParameter(std::unordered_map<std::string, std::string>& parames)
	{
		parames.clear();
		std::vector<std::string> tempArray1;
		std::vector<std::string> tempArray2;
		StringHelper::SplitString(this->mParamater, "&", tempArray1);
		for (const std::string & data : tempArray1)
		{
			StringHelper::SplitString(data, "=", tempArray2);
			if (tempArray2.size() != 2)
			{
				return false;
			}
			const std::string &key = tempArray2[0];
			const std::string &val = tempArray2[1];
			parames.insert(std::make_pair(key, val));
			GKDebugError("get parameter " << key << " = " << val);
		}
		return true;
	}

	void HttpRemoteGetRequestHandler::OnReceiveHeardAfter(XCode code)
	{
	}

	bool HttpRemoteGetRequestHandler::OnReceiveHeard(asio::streambuf & buf, size_t size)
	{
		return false;
	}
}