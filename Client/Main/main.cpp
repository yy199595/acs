#include"Object/App.h"
#include"Scene/LoggerComponent.h"
#include"Timer/TimerComponent.h"
#include"Coroutine/TaskComponent.h"
#include"Component/ClientComponent.h"
#include"Scene/ServiceMgrComponent.h"
#include"Scene/ThreadPoolComponent.h"
#include"Http/Component/HttpClientComponent.h"
using namespace Client;
using namespace Sentry;


int main(int argc, char ** argv)
{
    ComponentFactory::Add<TaskComponent>("TaskComponent");
    ComponentFactory::Add<TimerComponent>("TimerComponent");
    ComponentFactory::Add<ClientComponent>("ClientComponent");
    ComponentFactory::Add<LoggerComponent>("LoggerComponent");
    ComponentFactory::Add<HttpClientComponent>("HttpClientComponent");
    ComponentFactory::Add<ThreadPoolComponent>("ThreadPoolComponent");
    ComponentFactory::Add<ServiceMgrComponent>("ServiceMgrComponent");

    try
    {
        const std::string path(argv[1]);
        ServerConfig * serverConfig = new ServerConfig(path);
        return (new App(serverConfig))->Run(argc, argv);
    }
    catch(std::logic_error & err)
    {
        std::cerr << err.what() << std::endl;
        return -1;
    }
}