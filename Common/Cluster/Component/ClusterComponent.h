//
// Created by zmhy0073 on 2022/10/12.
//

#ifndef APP_CLUSTERCOMPONENT_H
#define APP_CLUSTERCOMPONENT_H
#include"Component/Component.h"

namespace Sentry
{
    class ClusterComponent : public Component, public IStart
    {
    public:
        ClusterComponent() = default;
        ~ClusterComponent() = default;
    private:
        bool Awake() final;
        bool Start() final;
    };
}


#endif //APP_CLUSTERCOMPONENT_H
