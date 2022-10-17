#include "core.h"

Engine Engine::Instance;

void Engine::Init()
{
    Render::Init();
    Logic::Init();
    Input::Init();
    Threads::Init();

    Engine::Run();
}

void Engine::Run()
{
    // Stasis::RefreshTime();

    // ThreadPool::Init();
    // Audio::Init();

    // AssetLoader::LoadAssets();
    // SceneLoader::LoadScene<SceneAudio1>();

    while (!Instance.quitRequested)
    {
        // Stasis::RefreshTime();

        // Travel();

        Render::Run();
        Input::Run();
        Logic::Run();

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

    Engine::Exit();
    Render::Exit();
    Logic::Exit();
    Input::Exit();
    Threads::Exit();
}

void Engine::Exit()
{
}

void Engine::Quit()
{
    Instance.quitRequested = true;
}
