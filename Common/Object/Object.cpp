#include"Object.h"
#include<Util/NumberHelper.h>

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
        if (!this->mIsActive)
        {
            this->SetActive(true);
            this->mClassName = name;
            this->mIntanceID = NumberHelper::Create();
            return true;
        }
        return false;
    }
}