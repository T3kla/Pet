#include "engine.h"

#include "input.h"
#include "logic.h"
#include "render.h"
#include "threads.h"

App *App::instance = nullptr;

App &App::Instance()
{
    return *instance;
}

App::App()
{
    LOG("\nInstance already exists \"%i\"", 3, "!", 29.f);

    if (instance)
        LOG("\nInstance already exists");

    instance = this;
}

void App::Init()
{
    threads.Init();

    render.Init();
    input.Init();
    logic.Init();

    Run();
}

void App::Run()
{
    // Stasis::RefreshTime();

    // ThreadPool::Init();
    // Audio::Init();

    // AssetLoader::LoadAssets();
    // SceneLoader::LoadScene<SceneAudio1>();

    while (!quitRequested)
    {
        // Stasis::RefreshTime();

        // Travel();

        input.Run();
        logic.Run();
        render.Run();

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
    render.Exit();
    logic.Exit();
    input.Exit();
    threads.Exit();
}

void App::Quit()
{
    quitRequested = true;
}
