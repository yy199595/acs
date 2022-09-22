#pragma once

#include"Log/CommonLogDef.h"
#include"Define/ClassStatement.h"

namespace Sentry
{
	class Object
	{
	 public:
		Object();

		virtual ~Object();

	 public:
		bool Init();

	 public:
		inline bool IsActive() const
		{
			return this->mIsActive;
		}

		inline void SetActive(bool isActive)
		{
			this->mIsActive = isActive;
		}

	 public:
		virtual void OnDestory() { }

	 private:
		bool mIsActive;
	};
}// namespace Sentry
