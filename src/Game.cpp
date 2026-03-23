#include "Game.h"
#include "Board.h"
#include "Utils.h"

#include <SDL3/SDL_messagebox.h>

Game::Game() :
    m_running(true),
    m_window(nullptr),
    m_renderer(nullptr)
{
}

Game::~Game()
{
}

bool Game::Initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return false;
    }
    
    if (!SDL_CreateWindowAndRenderer("Chess", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY, &m_window, &m_renderer))
    {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return false;
    }
    
#ifdef _WIN32
    if (!SDL_SetRenderVSync(m_renderer, 1))
    {
        SDL_Log("SDL_SetRenderVsync failed: %s", SDL_GetError());
        return false;
    }
#endif
    
    SDL_SetWindowAspectRatio(m_window, 1, 1);
    
    m_boardView.Init(m_renderer);
    m_board.Init(m_renderer, PIECE_COLOR_BLACK);
    m_menu.Init(m_window, m_renderer, m_board, m_boardView);
    return true;
}

void Game::Run()
{
    SDL_Event ev;
    
    bool needsRedraw = true;
    
    auto lastTime = SDL_GetTicks();
    while (m_running)
    {
        bool ret = SDL_WaitEventTimeout(&ev, 16);
        if (ret) HandleEvent(ev, needsRedraw);
        
        if (!m_running)
        {
            break;
        }

        auto now = SDL_GetTicks();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;

        // Updating
        switch (currentState)
        {
        case GameState::MainMenu:
        {
            Menu::Action act = m_menu.Update(m_window, m_board, dt);
            if (act == Menu::Action::StartGame)
            {
                currentState = GameState::InGame;
                m_board.Reset(true);
                needsRedraw = true;
            }
            else if (act == Menu::Action::Exit)
            {
                m_running = false;
                break;
            }
            break;
        }

        case GameState::InGame:
            m_boardView.Update(m_board, needsRedraw, dt);
            break;

        case GameState::Paused:
            // ???
            break;
        }

        // Rendering
        switch (currentState)
        {
        case GameState::MainMenu:
            SDL_SetRenderDrawColor(m_renderer, 128, 128, 128, 255);
            SDL_RenderClear(m_renderer);
            m_menu.Render(m_window, m_renderer, m_board, m_boardView);
            SDL_RenderPresent(m_renderer);
            break;
        
        case GameState::InGame:
            if (needsRedraw)
            {
                SDL_SetRenderDrawColor(m_renderer, 128, 128, 128, 255);
                SDL_RenderClear(m_renderer);
                m_boardView.Render(m_window, m_renderer, m_board);
                SDL_RenderPresent(m_renderer);
                needsRedraw = false;
            }
            break;
        
        case GameState::Paused:
            // ???
            break;
        }
    }
}

void Game::HandleEvent(SDL_Event& ev, bool& needsRedraw)
{
    if (ev.type == SDL_EVENT_QUIT)
    {
        m_running = false;
        return;
    }
    else if (ev.type == SDL_EVENT_WINDOW_RESIZED)
    {
        needsRedraw = true;
        return;
    }

    if (currentState == GameState::InGame)
    {
        bool isDone = m_boardView.HandleEvent(m_window, ev, m_board, needsRedraw);
        if (isDone)
        {
            currentState = GameState::MainMenu;
        }
    }
}
