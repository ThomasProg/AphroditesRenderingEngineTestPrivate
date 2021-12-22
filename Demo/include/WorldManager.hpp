#pragma once

#include <vector>

class World;

class WorldManager
{
    int currentWorldIndex = 0;
    std::vector<World*> worlds;

public:
    using WorldHandle = int;

    template<typename T>
    WorldHandle addWorld(class Window* w)
    {
        worlds.emplace_back(new T(w));
        return (int) worlds.size() - 1;
    }

    ~WorldManager();

    void setCurrentWorld(WorldHandle newWorld)
    {
        currentWorldIndex = newWorld;
    }

    World* getCurrent()
    {
        return worlds[currentWorldIndex];
    }
};