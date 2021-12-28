#include"Coroutine.h"

#ifdef _WIN32
#include<Windows.h>
#endif
namespace GameKeeper
{
	Coroutine::Coroutine()
    {
        memset(this, 0, (sizeof(Coroutine)));
    }

    void Coroutine::Invoke()
    {
        this->mState = Running;
        this->mFunction->run();
        if(this->mGroup != nullptr)
        {
            this->mGroup->FinishAny();
        }
        this->mGroup = nullptr;
        this->mState = CorState::Finish;
    }
	Coroutine::~Coroutine()
	{
        if(this->mStack.p)
        {
            free(this->mStack.p);
        }
	}
}