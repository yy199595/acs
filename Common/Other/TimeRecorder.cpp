#include "TimeRecorder.h"
#include <Util/DirectoryHelper.h>
#include <Util/MathHelper.h>
#include <Util/TimeHelper.h>
#include <fstream>

namespace Sentry
{
    TimeRecorder::TimeRecorder()
    {
        this->mLastClearTime = Helper::Time::GetMilTimestamp();
    }

    CostTimeInfo *TimeRecorder::GetLatencyInfo()
    {
        auto iter = this->mCallFuncInfoMap.begin();
        return iter != this->mCallFuncInfoMap.end() ? &iter->second : nullptr;
    }

    CostTimeInfo *TimeRecorder::GetLatencyInfo(std::string &func)
    {
        auto iter = this->mCallFuncInfoMap.find(func);
        return iter != this->mCallFuncInfoMap.end() ? &iter->second : nullptr;
    }

    void TimeRecorder::AddCostTimeInfo(const std::string &func, long long lastTime)
    {
        long long nowTime = Helper::Time::GetMilTimestamp();
        auto iter = this->mCallFuncInfoMap.find(func);
        if (iter == this->mCallFuncInfoMap.end())
        {
            CostTimeInfo pLatencyInfo;
            this->mCallFuncInfoMap.insert(std::make_pair(func, pLatencyInfo));
        }

        double delay = (nowTime - (lastTime * 1000)) / 1000.0f;

        CostTimeInfo &pLatencyInfo = this->mCallFuncInfoMap[func];

        pLatencyInfo.mCallCount++;
        pLatencyInfo.mSumLatency += delay;
        pLatencyInfo.mMaxLatency = Helper::Math::MaxZero(delay, pLatencyInfo.mMaxLatency);
        pLatencyInfo.mMinLatency = Helper::Math::MinZero(delay, pLatencyInfo.mMinLatency);
        pLatencyInfo.mAverageLatency = pLatencyInfo.mSumLatency / pLatencyInfo.mCallCount;
    }

    void TimeRecorder::AddCostTimeInfo(const std::string &func, XCode code, long long lastTime)
    {
        auto iter = this->mCallFuncInfoMap.find(func);
        if (iter == this->mCallFuncInfoMap.end())
        {
            CostTimeInfo pLatencyInfo;
            this->mCallFuncInfoMap.insert(std::make_pair(func, pLatencyInfo));
        }

        CostTimeInfo &pLatencyInfo = this->mCallFuncInfoMap[func];
        if (code == XCode::CallTimeout)
        {
            pLatencyInfo.mTimeoutCount++;
        } else
        {
            this->AddCostTimeInfo(func, lastTime);
        }
    }

    void TimeRecorder::Clear()
    {
        for (auto & iter : this->mCallFuncInfoMap)
        {
            CostTimeInfo &pLatencyInfo = iter.second;
            pLatencyInfo.Clear();
        }
        this->mLastClearTime = Helper::Time::GetMilTimestamp();
    }

    bool TimeRecorder::SaveDataToFile(const std::string path)
    {
        if (this->mCallFuncInfoMap.empty())
        {
            return false;
        }
        std::string fileName;
        std::string directory;
        if (!Helper::Directory::GetDirAndFileName(path, directory, fileName))
        {
            return false;
        }

        if (!Helper::Directory::DirectorIsExist(path))
        {
            Helper::Directory::MakeDir(directory);
        }

        std::fstream fs(path, std::ios::ate | std::ios::out | std::ios::in);
        if (!fs.is_open())
        {
            return false;
        }

        const std::string date = Helper::Time::GetDateString();
        for (auto iter = this->mCallFuncInfoMap.begin(); iter != this->mCallFuncInfoMap.end(); iter++)
        {
            const std::string &func = iter->first;
            CostTimeInfo &pLatencyInfo = iter->second;
            if (pLatencyInfo.mCallCount > 0)
            {
                fs << func << "\t" << pLatencyInfo.mCallCount << "\t" << pLatencyInfo.mMaxLatency << "\t"
                   << pLatencyInfo.mMinLatency << "\t" << pLatencyInfo.mAverageLatency << "\t"
                   << pLatencyInfo.mTimeoutCount << "\t" << date << "\n";
            }
        }
        fs.close();
        return true;
    }
}// namespace Sentry
