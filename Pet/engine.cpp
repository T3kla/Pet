#include "engine.h"

#include "input.h"
#include "logic.h"
#include "render.h"
#include "threads.h"

App *App::_instance = nullptr;

App *App::Instance()
{
    return _instance;
}

App::App()
{
    if (_instance)
        LOG("\nInstance already exists");

    _instance = this;
}

App::~App()
{
}

void App::Init()
{
    _threads.Init();

    _render.Init();
    _input.Init();
    _logic.Init();

    Run();
}

void App::Run()
{
    // Stasis::RefreshTime();

    // ThreadPool::Init();
    // Audio::Init();

    // AssetLoader::LoadAssets();
    // SceneLoader::LoadScene<SceneAudio1>();

    while (!_quitRequested)
    {
        // Stasis::RefreshTime();

        // Travel();

        _input.Run();
        _logic.Run();
        _render.Run();

        // dt = Stasis::GetDelta();
        // fxCount += dt;
        // fxCount = fmin(fxCount, STP * 2.);
        // while (fxCount >= STP)
        //{
        //     FreqRefresh(nowFx, oldFx, freqFx);

        //    Render::Fixed(); // FIXME: debug mode
        //    Logic::Fixed();
        //    Audio::Fixed();

        //    glfwSwapBuffers(Render::GetWindow()); // FIXME: debug mode
        //    fxCount -= STP;
        //}
    }

    // Audio::Exit();

    App::Exit();
}

void App::Exit()
{
    _render.Exit();
    _logic.Exit();
    _input.Exit();
    _threads.Exit();
}

void App::Quit()
{
    _quitRequested = true;
}
