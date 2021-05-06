#include"DownLoadManager.h"
#include<CommonUtil/MD5.h>

#include<CommonCore/Applocation.h>
#include<CommonUtil/FileHelper.h>
#include<CommonUtil/DirectoryHelper.h>
#include<CommonNetWork/RemoteScheduler.h>
#include<CommonOther/AssestsFileInfo.h>
#include<CommonCoroutine/CoroutineManager.h>
namespace SoEasy
{
	bool DownLoadManager::OnInit()
	{
		return true;
	}

	bool DownLoadManager::AsyncDownLoadAssest(shared_ptr<TcpClientSession> tcpSession, const std::string & assestName)
	{
		StringData fileName;
		fileName.set_data(assestName);
		TransferAssestInfo transferAssestInfo;
		
		XCode code = this->GetScheduler()->Call(tcpSession, "AssetsManager.DownLoadFile", &fileName, transferAssestInfo);
		if (code != XCode::Successful)
		{
			return false;
		}
		return true;
	}
}
