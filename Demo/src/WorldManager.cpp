#include "WorldManager.hpp"
#include "World.hpp"

WorldManager::~WorldManager()
{
    for (World* w : worlds)
    {
        delete w;
    }
}