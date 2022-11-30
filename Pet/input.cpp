#include "input.h"

#include "core.h"
#include "engine.h"
#include "render.h"

Input Input::Instance;

void Input::Init()
{
}

void Input::Run()
{
    glfwPollEvents();

    if (GLFW_PRESS == glfwGetKey(Render::GetWindow(), GLFW_KEY_ESCAPE))
        Engine::Quit();
}

void Input::Exit()
{
}
