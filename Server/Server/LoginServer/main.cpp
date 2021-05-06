
#include<CommonCore/Applocation.h>
#include<CommonManager/ListenerManager.h>
#include<CommonManager/AddressManager.h>
#ifdef _WIN32
#pragma comment(lib,"lua53.lib")
#pragma comment(lib,"Common.lib")
#pragma comment(lib,"libprotobufd.lib")
#endif
using namespace SoEasy;
int main(int argc, char ** argv)
{
	Applocation applocation("LoginServer");
	applocation.AddManager<ListenerManager>();
	applocation.AddManager<AddressManager>();

	return applocation.Run();
}