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

    bool HttpRemoteGetRequestHandler::ParseUrl(const std::string &path)
    {
        if (!HttpRemoteRequestHandler::ParseUrl(path))
        {
            return false;
        }
        size_t pos4 = path.find_last_of("/");
        size_t pos5 = path.find('?');
        if (pos5 == std::string::npos)
        {
            this->mParamater = path.substr(pos4 + 1);
            return true;
        }
        this->mParamater = path.substr(pos5 + 1);
        return true;
    }
}