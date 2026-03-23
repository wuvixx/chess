#include "Menu.h"
#include <SDL3/SDL.h>
#include <cstdio>

Menu::Menu() :
	m_font(nullptr)
{
}

Menu::~Menu()
{
	if (m_font)
	{
		// printf("Destroying font...\n");
		TTF_CloseFont(m_font);
		m_font = nullptr;
	}
}

void Menu::Init(SDL_Window* window, SDL_Renderer* renderer, Board& board, BoardView& boardView)
{
	if (!TTF_Init())
	{
		SDL_Log("Failed to initialize SDL_ttf: %s", SDL_GetError());
		return;
	}

	m_font = TTF_OpenFont("assets/fonts/Roboto-Regular.ttf", 24);
	if (!m_font)
	{
		SDL_Log("Failed to load font: %s", SDL_GetError());
		return;
	}

	// Initialize the board preview in the background
	board.Init(renderer, board.GetBottomPlayer());

	// Initialize buttons
	m_buttons[0].relX = 0.3f;
	m_buttons[0].relY = 0.6f;
	m_buttons[0].relW = 0.4f;
	m_buttons[0].relH = 0.1f;

	m_buttons[1].relX = 0.3f;
	m_buttons[1].relY = 0.8f;
	m_buttons[1].relW = 0.4f;
	m_buttons[1].relH = 0.1f;

	m_buttons[0].Init(renderer, "New game", m_font, (unsigned int)Action::StartGame);
	m_buttons[1].Init(renderer, "Quit", m_font, (unsigned int)Action::Exit);
}

Menu::Action Menu::Update(SDL_Window* window, Board& board, float delta)
{
	float mx, my;
	SDL_MouseButtonFlags flags = SDL_GetMouseState(&mx, &my);
	bool mouseDown = flags & SDL_BUTTON_LEFT;
	
	// Keep running background
	m_moveTimer += delta;
	if (m_moveTimer > 0.1f)
	{
		if (m_resetGameNext)
		{
			board.Reset(false);
			m_resetGameNext = false;
		}
		bool ret = board.AttemptRandomMove();
		if (!ret || board.GetTurn() == 200)
		{
			m_moveTimer = -5.0f;
			m_resetGameNext = true;
		}
		else
		{
			m_moveTimer = 0.0f;
		}
	}

	unsigned int buttonFlags = 0;
	for (int i = 0; i < sizeof(m_buttons) / sizeof(MenuButton); ++i)
	{
		buttonFlags |= m_buttons[i].Update(delta, mx, my, mouseDown);
	}

	if (buttonFlags & (unsigned int)Menu::Action::StartGame)
	{
		m_moveTimer = 0.0f;
		m_resetGameNext = false;
		return Menu::Action::StartGame;
	}
	else if (buttonFlags & (unsigned int)Menu::Action::Exit)
	{
		return Menu::Action::Exit;
	}
	return Menu::Action::None;
}


void Menu::Render(SDL_Window* window, SDL_Renderer* renderer, Board& board, BoardView& boardView)
{
	// Draw background board preview
	boardView.Render(window, renderer, board);

	// Dim the board with a translucent overlay
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
	SDL_RenderFillRect(renderer, nullptr);

	for (int i = 0; i < sizeof(m_buttons) / sizeof(MenuButton); ++i)
	{
		m_buttons[i].Render(renderer, boardView.GetOffsetX(), boardView.GetOffsetY(), boardView.GetBoardSize());
	}
}

void MenuButton::Init(SDL_Renderer* renderer, const char* text, TTF_Font* font, unsigned int flag)
{
	m_flag = flag;
	strcpy(m_label, text);
	SDL_Color color = {255, 255, 255, 255};
	SDL_Surface* surf = TTF_RenderText_Blended(font, m_label, sizeof(m_label), color);
	m_textTexture = SDL_CreateTextureFromSurface(renderer, surf);
	m_textW = surf->w;
	m_textH = surf->h;
	SDL_DestroySurface(surf);
}

void MenuButton::Destroy()
{
	if (m_textTexture)
	{
		SDL_DestroyTexture(m_textTexture);
		m_textTexture = nullptr;
	}
}

unsigned int MenuButton::Update(float delta, int mouseX, int mouseY, bool mouseDown)
{
	bool inside = (mouseX >= m_rect.x && mouseX <= m_rect.x + m_rect.w &&
				   mouseY >= m_rect.y && mouseY <= m_rect.y + m_rect.h);
	m_hovered = inside;

	if (m_hovered)
	{
		m_scale += (1.1f - m_scale) * 10.0f * delta;
		m_clicked = mouseDown;
		if (mouseDown)
		{
			return m_flag;
		}
	}
	else
	{
		m_scale += (1.0f - m_scale) * 10.0f * delta;
		m_clicked = false;
	}

	return 0;
}

void MenuButton::Render(SDL_Renderer* renderer, int offsetX, int offsetY, int boardSize)
{
	m_rect.x = offsetX + boardSize * relX;
	m_rect.y = offsetY + boardSize * relY;
	m_rect.w = boardSize * relW;
	m_rect.h = boardSize * relH;

	float scaledX = m_rect.x - (m_rect.w * (m_scale - 1.0f) / 2);
	float scaledY = m_rect.y - (m_rect.h * (m_scale - 1.0f) / 2);
	float scaledW = m_rect.w * m_scale;
	float scaledH = m_rect.h * m_scale;

	// Shadow
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 80);
	SDL_FRect shadow = { scaledX + 5, scaledY + 5, scaledW, scaledH };
	SDL_RenderFillRect(renderer, &shadow);

	// Button background
	SDL_SetRenderDrawColor(renderer, m_hovered ? 90 : 70, 70, 70, 255);
	SDL_FRect bg = { scaledX, scaledY, scaledW, scaledH };
	SDL_RenderFillRect(renderer, &bg);

	// Centered text
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_FRect textDst = {
		bg.x + (bg.w - m_textW) / 2,
		bg.y + (bg.h - m_textH) / 2,
		(float)m_textW,
		(float)m_textH};
	SDL_RenderTexture(renderer, m_textTexture, nullptr, &textDst);
}
