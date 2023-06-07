#include "input.h"

#include "core.h"
#include "engine.h"
#include "render.h"

void Input::Init()
{
}

void Input::Run()
{
    glfwPollEvents();

    auto *window = App::Instance().render.GetWindow();

    if (!window)
        return;

    if (GLFW_PRESS != glfwGetKey(window, GLFW_KEY_ESCAPE))
        return;

    App::Instance().Quit();
}

void Input::Exit()
{
}
