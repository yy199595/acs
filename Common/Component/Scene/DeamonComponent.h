//
// Created by zmhy0073 on 2021/10/29.
//

#ifndef GameKeeper_DEAMONCOMPONENT_H
#define GameKeeper_DEAMONCOMPONENT_H
#include "Component.h"
namespace GameKeeper
{
    class DeamonComponent : public Component, public ISecondUpdate
    {
    public:
        DeamonComponent() = default;
        ~DeamonComponent() = default;
    protected:
        bool Awake() override;
        void Start() override;
        void OnSecondUpdate() override;
    private:
        void Update();
        void ReadStdOut(const char * str, size_t);
        void ReadStdErr(const char * str, size_t);
    private:     
        long long mLastRefreshTime;
    };
}
#endif //GameKeeper_DEAMONCOMPONENT_H
