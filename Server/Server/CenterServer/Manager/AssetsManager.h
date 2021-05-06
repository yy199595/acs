#pragma once
#include<CommonManager/Manager.h>
#include<CommonOther/AssestsFileInfo.h>
#include<CommonProtocol/ServerCommon.pb.h>
namespace SoEasy
{
	class AssetsManager : public Manager
	{
	public:
		AssetsManager() { }
		~AssetsManager() { }
	protected:
		bool OnInit() override;
		void OnInitComplete() override;
	private:		
		XCode GetAssestsList(shared_ptr<TcpClientSession> session, long long, AssestCompareList & returnData);
		XCode DownLoadFile(shared_ptr<TcpClientSession> session, long long, const StringData & fineInfo, TransferAssestInfo & fileData);
	private:
		bool LoadAllAssest(const std::string & dir);
		AssestsFileInfo * GetAssestsByFullName(const std::string & name);
		void GetAssestListByName(const std::string & serverName, std::vector<std::string> & assestsNames);
	private:
		std::unordered_map<std::string, AssestsFileInfo *> mAllAssets;
	};
}