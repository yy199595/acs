#pragma once
#include "TelnetClientSession.h"
#include <Component/Component.h>
namespace GameKeeper
{
    class TelnetClientComponent : public Component
    {
    public:
        TelnetClientComponent();

        ~TelnetClientComponent();

    public:
        bool Awake() override;

    };
}

