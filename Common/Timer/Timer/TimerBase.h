﻿#pragma once

#include"Util/Tools/Guid.h"
#include"Util/Tools/TimeHelper.h"

namespace acs
{
    class TimerBase
    {
    public:
        friend class TimerComponent;
        explicit TimerBase(long long ms);

        virtual ~TimerBase() = default;

    public:
        virtual void Invoke() = 0;
    public:
		inline long long GetTimerId() const { return mTimerId; }
		inline void Refresh() { this->mTargetTime += this->mInterval; }
        inline long long GetTargetTime() const { return this->mTargetTime; }
	protected:
		long long mInterval;
		long long mTimerId;
		long long mTargetTime;
	};
}// namespace Sentry