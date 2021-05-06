
#include"Manager/RedisManager.h"
#include"Manager/MysqlManager.h"

#include<CommonCore/Applocation.h>
#include<CommonManager/AddressManager.h>
#include<CommonManager/ListenerManager.h>
#ifdef _WIN32
#pragma comment(lib,"lua53.lib")
#pragma comment(lib,"Common.lib")
#pragma comment(lib,"hiredis.lib")
#pragma comment(lib,"libmysql.lib")
#pragma comment(lib,"libprotobufd.lib")
#endif


using namespace SoEasy;
int main(int argc, char ** argv)
{
	Applocation appLocation("DataServer");
	appLocation.AddManager<MysqlManager>();
	appLocation.AddManager<ListenerManager>();
	appLocation.AddManager<AddressManager>();

	return appLocation.Run();

}