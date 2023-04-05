//
// Created by 64658 on 2023/4/5.
//

#ifndef APP_EVENTMGRCOMPONENT_H
#define APP_EVENTMGRCOMPONENT_H
#include<unordered_map>
#include"Core/Component/Component.h"

namespace Tendo
{
    class EventMgrComponent : public Component
    {
    public:

    private:
        std::unordered_map<std::string, std::string> mEvents;
    };

}
#endif //APP_EVENTMGRCOMPONENT_H
