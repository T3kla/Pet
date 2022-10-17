#include "core.h"

Input Input::Instance;

void Input::Init()
{
}

void Input::Run()
{
    glfwPollEvents();

    auto *window = Render::GetWindow();

    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
        Engine::Quit();
}

void Input::Exit()
{
}
