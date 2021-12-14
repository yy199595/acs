#include<Core/App.h>
#include"ClientComponent.h"
#include"Scene/LoggerComponent.h"
#include"Timer/TimerComponent.h"
#include"Scene/ThreadPoolComponent.h"
#include"Coroutine/TaskComponent.h"

using namespace Client;
using namespace GameKeeper;


int main(int argc, char ** argv)
{
	REGISTER_COMPONENT(TimerComponent);
	REGISTER_COMPONENT(LoggerComponent);
	REGISTER_COMPONENT(ThreadPoolComponent);
	REGISTER_COMPONENT(TaskComponent);

	REGISTER_COMPONENT(ClientComponent);

	if (argc == 1)
	{
		argv[1] = new char[100];
		argv[1] = "./Config/client.json";
	}
	App app;
	return app.Run(argc, argv);
}