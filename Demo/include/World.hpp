#pragma once

class World
{

public:
    World() = default;
    virtual void update() = 0;
    virtual ~World() = default;
};