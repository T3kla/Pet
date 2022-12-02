#include "engine.h"

#include <exception>
#include <iostream>

int main()
{
    auto engine = App();

    try
    {
        engine.Init();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
