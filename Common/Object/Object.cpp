#include"Object.h"
#include<Core/App.h>
#include<Util/NumberHelper.h>
#include<Coroutine/CoroutineComponent.h>

namespace Sentry
{
    Object::Object()
    {
        this->SetActive(false);
    }

    Object::~Object()
    {

    }

    bool Object::Init(const std::string &name)
    {
        if (this->mIsActive == false)
        {
            this->SetActive(true);
            this->mClassName = name;
            this->mIntanceID = NumberHelper::Create();
            return true;
        }
        return false;
    }
}