#pragma once
#include <SDL3/SDL.h>

void DrawCircle(SDL_Renderer* renderer, float cx, float cy, float radius);
bool IsPointInCircle(float x, float y, float cx, float cy, float radius);
