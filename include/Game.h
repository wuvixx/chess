#pragma once
#include "Menu.h"
#include "Board.h"
#include "BoardView.h"

enum class GameState
{
    MainMenu,
    InGame,
    Paused
};

class Game
{
public:
    Game();
    ~Game();

    bool Initialize();
    void Run();
    // void ShowMeSquares();

    void ShowCheckmateMessage(PieceColor winner);
    void ShowStalemateMessage();

private:
    GameState currentState = GameState::MainMenu;
    bool m_running;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    
    Menu m_menu;
    Board m_board;
    BoardView m_boardView;
    
    void HandleEvent(SDL_Event& ev, bool& needsRedraw);
};
