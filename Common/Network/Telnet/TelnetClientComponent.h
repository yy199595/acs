#pragma once
#include "TelnetClientSession.h"
#include <Component/Component.h>
namespace GameKeeper
{
    class TelnetClientComponent : public Component
    {
    public:
        TelnetClientComponent() = default;

        ~TelnetClientComponent() final = default;

    public:
        bool Awake() final;
        bool LateAwake() final;
    };
}

