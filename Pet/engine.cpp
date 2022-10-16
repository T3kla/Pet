#include "core.h"

Engine Engine::Instance;

void Engine::Init()
{
    std::cout << "Engine::Init()" << std::endl;

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

    while (!Instance.closeRequested)
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

    Engine::Exit();

    // Audio::Exit();
    // ThreadPool::Exit();
}

void Engine::Exit()
{
    Render::Exit();
    Logic::Exit();
    Input::Exit();
    Threads::Exit();
}

void Engine::CloseRequest()
{
    Instance.closeRequested = true;
}