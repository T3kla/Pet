#include "input.h"

#include "core.h"
#include "engine.h"
#include "render.h"

Input *Input::_instance;

Input *Input::Instance()
{
    return _instance;
}

Input::Input()
{
    if (_instance)
        LOG("\nInstance already exists");

    _instance = this;
}

Input::~Input()
{
}

void Input::Init()
{
}

void Input::Run()
{
    glfwPollEvents();

    auto *render = Render::Instance();

    if (!render)
        return;

    auto *window = Render::Instance()->GetWindow();

    if (!window)
        return;

    if (GLFW_PRESS != glfwGetKey(window, GLFW_KEY_ESCAPE))
        return;

    auto *app = App::Instance();

    if (!app)
        return;

    app->Quit();
}

void Input::Exit()
{
}
