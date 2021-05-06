#include "AssetsManager.h"
#include<regex>
#include<CommonCore/Applocation.h>
#include<CommonUtil/DirectoryHelper.h>
#include<CommonManager/AddressManager.h>
#include<CommonNetWork/RemoteScheduler.h>
#include<CommonOther/ServerRegisterInfo.h>
namespace SoEasy
{

	bool AssetsManager::OnInit()
	{
		std::string nAssetsPath;
		ServerConfig & nServerConfig = this->GetConfig();
		if (!nServerConfig.GetValue("DownLoadAssetsPath", nAssetsPath))
		{
			SayNoDebugError("not find field : AssetsPath");
			return false;
		}
		if (!this->LoadAllAssest(nAssetsPath))
		{
			return false;
		}
		return true;
	}

	void AssetsManager::OnInitComplete()
	{
		REGISTER_FUNCTION_1(AssetsManager::GetAssestsList, AssestCompareList);
		REGISTER_FUNCTION_2(AssetsManager::DownLoadFile, StringData, TransferAssestInfo);
	}

	XCode AssetsManager::GetAssestsList(shared_ptr<TcpClientSession> session, long long, AssestCompareList & returnData)
	{
		std::vector<std::string> mAssestsNames;
		this->GetAssestListByName(session->GetSessionName(), mAssestsNames);
		
		std::sort(mAssestsNames.begin(), mAssestsNames.end(), []
			(const std::string & s1,const std::string & s2)->bool
		{
			return s1.size() < s2.size();
		});

		for (std::string & fullName : mAssestsNames)
		{
			AssestsFileInfo * nAssest = this->GetAssestsByFullName(fullName);
			if (nAssest != nullptr)
			{
				auto  pAssestInfo = returnData.add_assestlist();
				pAssestInfo->set_file_name(fullName);
				pAssestInfo->set_file_md5(nAssest->GetMd5());
			}
		}
		return XCode::Successful;
	}

	XCode AssetsManager::DownLoadFile(shared_ptr<TcpClientSession> session, long long, const StringData & fileName, TransferAssestInfo & fileData)
	{
		AssestsFileInfo * pFileInfo = this->GetAssestsByFullName(fileName.data());
		if (pFileInfo == nullptr)
		{
			return XCode::AssestNotExist;
		}
		fileData.set_file_name(pFileInfo->GetFullName());
		fileData.set_file_content(pFileInfo->GetFileContent());
		SayNoDebugLog(session->GetSessionName() << " down load " << fileName.data());
		return XCode::Successful;
	}

	bool AssetsManager::LoadAllAssest(const std::string & dir)
	{
		std::vector<std::string> nResourcesPaths;
		if (!DirectoryHelper::GetFilePaths(dir, nResourcesPaths))
		{
			return false;
		}

		for (auto iter = this->mAllAssets.begin(); iter != this->mAllAssets.end(); iter++)
		{
			AssestsFileInfo * fileInfo = iter->second;
			delete fileInfo;
		}
		this->mAllAssets.clear();

		ServerConfig & nConfig = this->GetConfig();
		for (size_t index = 0; index < nResourcesPaths.size(); index++)
		{
			const std::string & path = nResourcesPaths[index];
			unsigned int nbufferCount = nConfig.GetSendBufferCount();
			AssestsFileInfo * fileInfo = new AssestsFileInfo();
			if (fileInfo->LaodFile(dir, path) == false)
			{
				SayNoDebugError("load " << path << " fail");
				delete fileInfo;
				return false;
			}

			if (fileInfo->GetFileSize() == 0)
			{
				delete fileInfo;
				continue;
			}

			const std::string & name = fileInfo->GetFullName();
			this->mAllAssets.insert(std::make_pair(name, fileInfo));
			SayNoDebugInfo("load assest file : " << name);
		}
		return true;
	}

	void AssetsManager::GetAssestListByName(const std::string & serverName, std::vector<std::string> & assestsNames)
	{
		assestsNames.clear();
		for (auto iter = this->mAllAssets.begin(); iter != this->mAllAssets.end(); iter++)
		{
			const std::string & fullName = iter->first;
			size_t pos1 = fullName.find("Common/");
			size_t pos2 = fullName.find(serverName + "/");
			if (pos1 != std::string::npos || pos2 != std::string::npos)
			{
				assestsNames.push_back(fullName);
			}
		}
	}

	AssestsFileInfo * AssetsManager::GetAssestsByFullName(const std::string & name)
	{
		auto iter = this->mAllAssets.find(name);
		return iter != this->mAllAssets.end() ? iter->second : nullptr;
	}

}
