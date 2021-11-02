//
// Created by zmhy0073 on 2021/11/1.
//
#include <regex>
#include "HttpRemoteGetRequest.h"
#include <Util/StringHelper.h>
#include <Define/CommonDef.h>
namespace GameKeeper
{
	bool HttpRemoteGetRequest::SplitParameter(std::unordered_map<std::string, std::string>& parames)
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

	bool HttpRemoteGetRequest::OnReceiveBody(asio::streambuf &buf)
    {
		return false;
    }

    bool HttpRemoteGetRequest::OnReceiveHeard(asio::streambuf &buf, size_t size)
    {
        std::string path;
        std::istream is(&buf);
        is >> path >> this->mVersion;
		this->ParseHeard(buf, size - (size - buf.size()));
       
		if (!this->ParseUrl(path))
		{
			this->SetCode(HttpStatus::NOT_FOUND);
			return false;
		}  
        return this->NoticeMainThread();
    }
	bool HttpRemoteGetRequest::ParseUrl(const std::string & path)
	{
		const static std::string & app = "App/";
		size_t pos1 = path.find(app, 1);
		if (pos1 == std::string::npos)
		{
			return false;
		}
		size_t pos2 = path.find("/", pos1 + app.size());
		if (pos2 == std::string::npos)
		{
			return false;
		}
		size_t pos3 = path.find("/", pos2);
		size_t pos4 = path.find("/", pos3 + 1);
		if (pos3 == std::string::npos || pos3 == std::string::npos)
		{
			return false;
		}
		this->mService = path.substr(pos1 + app.size(), pos2 - app.size() - 1);
		this->mMethod = path.substr(pos3 + 1, pos4 - pos3 - 1);

		size_t pos5 = path.find("?");
		if (pos5 == std::string::npos)
		{
			this->mParamater = path.substr(pos4 + 1);
			return true;
		}
		this->mParamater = path.substr(pos5 + 1);
		return true;
	}
}