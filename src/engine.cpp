#include "engine.h"

#include <iostream>
#include "core.h"

Engine Engine::Instance;

void Engine::Init()
{
    v2f v;
    v.x = 4;
    std::cout << v << std::endl;
}
