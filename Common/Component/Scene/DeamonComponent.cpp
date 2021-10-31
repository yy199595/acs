//
// Created by zmhy0073 on 2021/10/29.
//


#include "DeamonComponent.h"
#include <Util/TimeHelper.h>
namespace Sentry
{
    bool DeamonComponent::Awake()
    {
        return true;
    }

    void DeamonComponent::Start()
    {

    }

    void DeamonComponent::OnSecondUpdate()
    {

    }

    void DeamonComponent::ReadStdOut(const char *str, size_t size)
    {
        //SayNoDebugWarning(std::string(str, size) << "  " << this->mLastRefreshTime);
    }

    void DeamonComponent::ReadStdErr(const char *str, size_t)
    {

    }

    void DeamonComponent::Update()
    {
        std::chrono::seconds t(1);
        while(true)
        {
            std::this_thread::sleep_for(t);

//            char buffer[1024] = {0};
//            fgets(buffer, 1024, stdin);
//
//            if(buffer[0] !=0)
//            {
//                std::cout << buffer << std::endl;
//            }

           // std::cout << TimeHelper::GetDateString() << std::endl;
            //size_t size = std::cin.readsome(buffer, 1024);

//            std::cout << "last refresh time = " << std::string(buffer, size)
//                      << "  thread id = " << std::this_thread::get_id() << std::endl;
        }
        std::cout << "quit process " << std::endl;
    }
}