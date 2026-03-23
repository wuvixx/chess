#define SDL_MAIN_USE_CALLBACKS 1
#include "Game.h"
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#if defined(_WIN32) && !defined(DEBUG)
int WinMain()
#else
    int main()
#endif
{
    Game game;
    game.Initialize();
    game.Run();
}
