#include "logic.h"

#include "core.h"

Logic *Logic::_instance;

Logic *Logic::Instance()
{
    return _instance;
}

Logic::Logic()
{
    if (_instance)
        LOG("\nInstance already exists");

    _instance = this;
}

Logic::~Logic()
{
}

void Logic::Init()
{
}

void Logic::Run()
{
}

void Logic::Exit()
{
}
