#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "Board.h"
#include "BoardView.h"

struct MenuButton
{
public:
    float relX, relY, relW, relH;

    void Init(SDL_Renderer* renderer, const char* text, TTF_Font* font, unsigned int flag);
    void Destroy();
    unsigned int Update(float delta, int mouseX, int mouseY, bool mouseDown);
    void Render(SDL_Renderer* renderer, int offsetX, int offsetY, int boardSize);
private:
    SDL_FRect m_rect;
    SDL_Texture* m_textTexture = nullptr;

    unsigned int m_flag = 0;

    char m_label[56]{};
    int m_textW = 0, m_textH = 0;

    bool m_hovered = false;
    bool m_clicked = false;

    float m_scale = 1.0f;
};

class Menu
{
public:
    Menu();
    ~Menu();
    enum class Action
    {
        None = 0,
        StartGame = 1 << 0,
        Exit = 1 << 1
    };

    void Init(SDL_Window* window, SDL_Renderer* renderer, Board& board, BoardView& boardView);
    // Update returns an action (StartGame / Exit) when the user clicks a button or uses keyboard shortcuts
    Action Update(SDL_Window* window, Board& board, float delta);
    void Render(SDL_Window* window, SDL_Renderer* renderer, Board& board, BoardView& boardView);

private:
    float m_moveTimer = 0.0f;
    bool m_resetGameNext = false;

    TTF_Font* m_font;

    // Buttons
    MenuButton m_buttons[2];
};