#include"Object.h"
#include<Util/Guid.h>

namespace Sentry
{
    Object::Object()
    {
        this->SetActive(false);
    }

    Object::~Object()
    {

    }

    bool Object::Init()
    {
        if (!this->mIsActive)
        {
            this->SetActive(true);
            return true;
        }
        return false;
    }
}