#include "core.h"

Engine Engine::Instance;

void Engine::Init()
{
    std::cout << "Engine::Init()" << std::endl;

    Render::Init();
}
