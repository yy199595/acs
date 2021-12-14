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
	Coroutine::~Coroutine()
	{
        if(this->mStack.p)
        {
            free(this->mStack.p);
        }
	}
}