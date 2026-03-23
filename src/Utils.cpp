#include "Utils.h"

void DrawCircle(SDL_Renderer* renderer, float cx, float cy, float radius)
{
    for (int w = -radius; w <= radius; ++w)
    {
        for (int h = -radius; h <= radius; ++h)
        {
            if (w*w + h*h <= radius*radius)
            {
                SDL_RenderPoint(renderer, cx + w, cy + h);
            }
        }
    }
    
    // for (float x = cx - radius; x <= cx + radius; ++x)
    // {
    //     for (float y = cy - radius; y <= cy + radius; ++y)
    //     {
    //         float dx = x - cx;
    //         float dy = y - cy;
    //         if (dx*dx + dy*dy <= radius*radius)
    //         {
    //             SDL_RenderPoint(renderer, x, y);
    //         }
    //     }
    // }
}

bool IsPointInCircle(float x, float y, float cx, float cy, float radius)
{
    int dx = x - cx;
    int dy = y - cy;
    if (dx*dx + dy*dy <= radius*radius)
    {
        return true;
    }
    return false;
}
