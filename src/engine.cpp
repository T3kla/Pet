#include "engine.h"

#include <iostream>
#include "core.h"

Engine Engine::Instance;

void Engine::Init()
{
    v2f v = {4, 5};
    std::cout << v << std::endl;
}
