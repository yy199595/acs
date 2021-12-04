#include<Core/App.h>
#include"ClientComponent.h"
#include"Scene/LoggerComponent.h"
#include"Timer/TimerComponent.h"
#include"Scene/TaskPoolComponent.h"
#include"Coroutine/CoroutineComponent.h"

using namespace Client;
using namespace GameKeeper;


int main(int argc, char ** argv)
{
#ifdef __DEBUG__ && _WIN32
	system("chcp 936");
#endif // __DEBUF__ && _WIN32

	__register_component__(TimerComponent);
	__register_component__(LoggerComponent);
	__register_component__(TaskPoolComponent);
	__register_component__(CoroutineComponent);

	__register_component__(ClientComponent);

	if (argc == 1)
	{
		argv[1] = new char[100];
		argv[1] = "./Config/client.json";
	}
	App app;
	return app.Run(argc, argv);
}