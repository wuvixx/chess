#pragma once
#include "Board.h"
#include "Utils.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

struct FlashEffect
{
    Piece* piece;

    // Time
    Uint32 startTime;
    Uint32 duration;

    bool visible, active;
};

struct SoundChunk
{
    SDL_AudioSpec spec;
    MIX_Mixer* mixer;
    MIX_Audio* gMove;
    MIX_Audio* gCapture;
};

struct Button
{
    int row;
    int col;
    SDL_FRect rect, optRect, pieceTexRect;
    SDL_Texture* pieceTex;

    // ColorMod
    Uint8 r = 255, g = 255, b = 255;
    
    // Scale
    float pieceTexScale = 1.0f;
    float optTexScale = 1.0f;
    float colorModScale = 1.0f;
    
    // State
    bool active = false;
    bool hovered = false;
    bool isAnimating = false;
};

class BoardView
{
public:
    BoardView();
    ~BoardView();

    bool Init(SDL_Renderer* renderer);
    bool HandleEvent(SDL_Window* window, SDL_Event& ev, Board& board, bool& needsRedraw);
    void Update(Board& board, bool& needsRedraw, float delta);

    // For debugging
    void ShowMeSquares(SDL_Renderer* renderer, Board& board);

    void Render(SDL_Window *window, SDL_Renderer *renderer, Board &board);

    void DrawBoard(SDL_Window* window, SDL_Renderer* renderer, Board& board, float windowWidth, float windowHeight);
    void DrawPieces(SDL_Renderer* renderer, Board& board, float squareSize, float offsetX, float offsetY);

    
    // Pixel stuff
    Piece* GetPieceAt(Board& board, float x, float y) const;
    SDL_FRect GetSquareRect(int row, int col) const { return { m_offsetX + col * m_squareSize, m_offsetY + row * m_squareSize, m_squareSize, m_squareSize }; };
    bool PixelToSquare(Board& board, float x, float y, int &outRow, int &outCol) const;
    
    // Getters
    float GetBoardSize() const { return m_boardSize; }
    float GetSquareSize() const { return m_squareSize; };
    float GetOffsetX() const { return m_offsetX; }
    float GetOffsetY() const { return m_offsetY; }

private:
    void ShowCheckmateMessage(SDL_Window* window, PieceColor winner);
    void ShowStalemateMessage(SDL_Window* window);

private:
    void ShowPromotionOptions(Piece* promotingPawn, Board& board);
    void HandlePawnPromotion(Piece* pawn, Board& board, bool& needsRedraw);
    
    private:
    // Render methods
    static void SetPixel(SDL_Surface* surface, int x, int y, Uint32 color);
    static SDL_Texture* CreateGradientCircle(SDL_Renderer* renderer, int radius, Uint8 edgeColor, Uint8 centerColor);
    
    SDL_Rect ClipFromFRect(const SDL_FRect &f);
    
    // Pieces
    bool LoadPieceTextures(SDL_Renderer* renderer);
    void DestroyPieceTextures();
    SDL_Texture* GetPieceTexture(PieceType type, PieceColor color) const;
    void DrawPiece(SDL_Renderer* renderer, Piece* piece) const;
    void DrawPieceAt(SDL_Renderer* renderer, Piece* piece, float x, float y);
    
    // DISABLED: Not used in rendering, kept as an example of interpolation over surface pixels
    static SDL_Texture* TestTexture(SDL_Renderer* renderer, int radius);
    
    // Window stuff
    float m_boardSize, m_squareSize, m_offsetX, m_offsetY;
    
    // Mouse states
    float m_mouseX, m_mouseY;
    bool m_isMousePointing, m_isMouseDraggingPiece;
    
    // Visual piece states
    float m_lastHoverOffsetX, m_lastHoverOffsetY;
    Piece* m_lastHoveredPiece;
    
    // Mouse cursors
    mutable SDL_Cursor* m_cursorDefault;
    mutable SDL_Cursor* m_cursorMove;
    mutable SDL_Cursor* m_cursorPointer;
    
    // Flashing effect
    void StartFlashingPiece(Piece* piece);
    void UpdateFlashing(bool& needsRedraw);
    FlashEffect m_flashEffect;

    // Promotion
    SDL_Texture* m_promotionCircle;
    bool m_showingPromotion;
    PieceType m_promotionMouseOver;

    // Sound
    SoundChunk m_soundChunk;

    // Buttons
    Button m_buttons[4];

    // Test
    SDL_Texture* m_testTexture;

    static SDL_Texture* s_circle;
    static SDL_Texture* s_textures[2][6];
};
