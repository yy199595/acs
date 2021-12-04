#include"Object.h"
#include<Util/NumberHelper.h>

namespace GameKeeper
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
            return true;
        }
        return false;
    }
}