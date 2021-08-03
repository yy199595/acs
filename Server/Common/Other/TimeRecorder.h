#pragma once

#include <XCode/XCode.h>
#include <cstring>
#include <unordered_map>

namespace Sentry
{
    struct CostTimeInfo {
    public:
        CostTimeInfo() { this->Clear(); }

    public:
        void Clear() { memset(this, 0, sizeof(*this)); }

    public:
        int mTimeoutCount;     //超时次数
        long long mCallCount;  //调用次数
        double mSumLatency;    //总延迟
        double mMaxLatency;    //最大延迟
        double mMinLatency;    //最小延迟
        double mAverageLatency;//平均延迟
    };


    class TimeRecorder
    {
    public:
        TimeRecorder();

    public:
        CostTimeInfo *GetLatencyInfo();

        CostTimeInfo *GetLatencyInfo(std::string &name);

        long long GetLastClearTime() { return this->mLastClearTime; }

    public:
        void Clear();

        bool SaveDataToFile(const std::string path);

        void AddCostTimeInfo(const std::string &func, long long lastTime);

        void AddCostTimeInfo(const std::string &func, XCode code, long long lastTime);

    private:
        long long mLastClearTime;//(毫秒)
        std::unordered_map<std::string, CostTimeInfo> mCallFuncInfoMap;
    };
}// namespace Sentry