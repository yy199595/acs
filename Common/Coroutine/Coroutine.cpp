#include"Coroutine.h"

#ifdef _WIN32
#include<Windows.h>
#endif
namespace GameKeeper
{
	Coroutine::Coroutine()
	{
        memset(this, 0, (sizeof(Coroutine)));
        this->mState = CorState::Ready;
	}

    void Coroutine::Invoke()
    {
        this->mState = Running;
        this->mFunction->run();
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