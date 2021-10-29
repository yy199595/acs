//
// Created by zmhy0073 on 2021/10/29.
//

#ifndef SENTRY_DEAMONCOMPONENT_H
#define SENTRY_DEAMONCOMPONENT_H
#include "Component.h"
#include <Process/process.hpp>
namespace Sentry
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
        Sentry::Process * mDeamon;
        long long mLastRefreshTime;
    };
}
#endif //SENTRY_DEAMONCOMPONENT_H
