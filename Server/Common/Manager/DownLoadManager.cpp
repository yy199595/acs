#include"DownLoadManager.h"
#include<Util/MD5.h>
#include<Util/FileHelper.h>
#include<Core/Applocation.h>
#include<Util/DirectoryHelper.h>
#include<Other/AssestsFileInfo.h>
#include<NetWork/RemoteScheduler.h>
#include<Coroutine/CoroutineManager.h>
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
