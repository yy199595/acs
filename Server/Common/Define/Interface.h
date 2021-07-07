#pragma once

class IAwake
{
public:
    virtual void OnAwake() = 0;
};

class IUpdate
{
public:
    virtual void OnUpdate(float t) = 0;
};

class ISystemUpdate
{
public:
    virtual void OnSystemUpdate() = 0;
};