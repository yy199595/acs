
#include<CommonCore/Applocation.h>
#include"Manager/ProxySrvServerManager.h"
#include"Manager/ProxySrvClientManager.h"

#ifdef _WIN32
#pragma comment(lib,"Common.lib")
#pragma comment(lib,"libprotobufd.lib")
#endif


using namespace SayNo;
int main(int argc, char ** argv)
{

	Applocation * frameWork = new Applocation("ProxySrv");
	SayNoAssertBreakFatal_F(frameWork->AddManager<ProxySrvServerManager>());
	SayNoAssertBreakFatal_F(frameWork->AddManager<ProxySrvClientManager>());
	
	return frameWork->Run();
}