//
// Created by yjz on 2022/1/5.
//

#ifndef GAMEKEEPER_TASKSOURCE_H
#define GAMEKEEPER_TASKSOURCE_H
#include"WaitTaskSourceBase.h"
namespace Tendo
{
    class LoopTaskSource : public WaitTaskSourceBase
    {
    public:
        void Await() {this->YieldTask();}
        bool SetResult() { return this->ResumeTask(TaskState::TaskReady);}
    };

    template<typename T>
    class TaskSource : public WaitTaskSourceBase
    {
    public:
        const T & Await();
        bool SetResult(const T & data);
    private:
        T mData;
    };

    template<typename T>
    const T & TaskSource<T>::Await()
    {
        this->YieldTask();
        return this->mData;
    }

    template<typename T>
    bool TaskSource<T>::SetResult(const T &data)
    {
        if(this->ResumeTask())
        {
            this->mData = data;
            return true;
        }
        return false;
    }
}

namespace Tendo
{
	template<typename T>
	class PtrTaskSource : public WaitTaskSourceBase
	{
	 public:
		std::shared_ptr<T> Await();
		bool SetResult(std::shared_ptr<T> data);
	 private:
		std::shared_ptr<T> mData;
	};

	template<typename T>
	bool PtrTaskSource<T>::SetResult(std::shared_ptr<T> data)
	{
		if(this->ResumeTask())
		{
			this->mData = data;
			return true;
		}
		return false;
	}

	template<typename T>
	std::shared_ptr<T> PtrTaskSource<T>::Await()
	{
		this->YieldTask();
		return this->mData;
	}
}

namespace Tendo
{
    template<> class TaskSource<void> : public WaitTaskSourceBase
    {
    public:
        void Await() {this->YieldTask();}
        bool SetResult() { return this->ResumeTask();}
    };
}

namespace Tendo
{
    template<typename T>
    class TaskSource<T *> { };
    template<typename T>
    class TaskSource<T &> { };
    template<typename T>
    class TaskSource<const T *> { };
    template<typename T>
    class TaskSource<const T &> { };
}

namespace Tendo
{
    template<typename T>
    class TaskSource<std::shared_ptr<T>> : public WaitTaskSourceBase
    {
    public:
        std::shared_ptr<T> Await();
        bool SetResult(std::shared_ptr<T> data);
    private:
        std::shared_ptr<T> mData;
    };
    template<typename T>
    std::shared_ptr<T> TaskSource<std::shared_ptr<T>>::Await()
    {
        this->YieldTask();
        return this->mData;
    }
    template<typename T>
    bool TaskSource<std::shared_ptr<T>>::SetResult(std::shared_ptr<T> data)
    {
		this->mData = data;
		return this->ResumeTask();
    }
}

#endif //GAMEKEEPER_TASKSOURCE_H
