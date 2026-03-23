#include "BoardView.h"
#include "Board.h"
#include <cmath>
#include <iostream>

SDL_Texture* BoardView::s_textures[2][6] = {{ nullptr }};
SDL_Texture* BoardView::s_circle = nullptr;

BoardView::BoardView()
    :
    m_boardSize(0.0f),
    m_squareSize(0.0f), m_offsetX(0.0f), m_offsetY(0.0f),
    m_mouseX(0.0f), m_mouseY(0.0f),
    m_isMousePointing(false), m_isMouseDraggingPiece(false),
    m_lastHoverOffsetX(0.0f), m_lastHoverOffsetY(0.0f),
    m_lastHoveredPiece(nullptr),
    m_cursorDefault(nullptr), m_cursorMove(nullptr), m_cursorPointer(nullptr),
    m_flashEffect{},
    m_showingPromotion(false), m_promotionMouseOver(PIECE_NONE), m_promotionCircle(nullptr),
    m_soundChunk{}, m_testTexture(nullptr)
{
}

BoardView::~BoardView()
{
    // printf("Destroying mouse cursors...\n");
    if (m_cursorDefault) SDL_DestroyCursor(m_cursorDefault);
    if (m_cursorPointer) SDL_DestroyCursor(m_cursorPointer);
    if (m_cursorMove) SDL_DestroyCursor(m_cursorMove);

    // printf("Destroying sound chunks...\n");
    if (m_soundChunk.gMove) MIX_DestroyAudio(m_soundChunk.gMove);
    if (m_soundChunk.gCapture) MIX_DestroyAudio(m_soundChunk.gCapture);
    if (m_soundChunk.mixer) MIX_DestroyMixer(m_soundChunk.mixer);

    // printf("Destroying board view textures...\n");
    DestroyPieceTextures();
    if (s_circle) SDL_DestroyTexture(s_circle);
    if (m_promotionCircle) SDL_DestroyTexture(m_promotionCircle);
}

bool BoardView::Init(SDL_Renderer* renderer)
{
    // Cursors
    m_cursorDefault = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    m_cursorMove = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
    m_cursorPointer = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);

    // Sounds
    if (!MIX_Init())
    {
        SDL_Log("Mix_Init failed: %s", SDL_GetError());
    }

    m_soundChunk.spec = { SDL_AUDIO_S16, 2, 48000 };
    m_soundChunk.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &m_soundChunk.spec);
    m_soundChunk.gMove = MIX_LoadAudio(m_soundChunk.mixer, "assets/sounds/move-self.mp3", true);
    m_soundChunk.gCapture = MIX_LoadAudio(m_soundChunk.mixer, "assets/sounds/capture.mp3", true);
    if (!m_soundChunk.gMove)
    {
        std::cout << "Couldn't load gMove!!!!" << std::endl;
    }
    if (!m_soundChunk.gCapture)
    {
        std::cout << "Couldn't love gCapture!!!!" << std::endl;
    }

    // Textures
    bool success = LoadPieceTextures(renderer);
    if (!success)
    {
        std::cout << "Couldn't load textures!!!!" << std::endl;
        return false;
    }

    if (!s_circle)
    {
        SDL_Surface* circleSurf = IMG_Load("assets/pieces/circle.png");
        if (!circleSurf)
        {
            SDL_Log("Couldn't load circle surf: %s", SDL_GetError());
            return false;
        }
        s_circle = SDL_CreateTextureFromSurface(renderer, circleSurf);
        SDL_SetTextureAlphaMod(s_circle, 100);
    }

    m_promotionCircle = CreateGradientCircle(renderer, 64, 147, 255);
    // m_testTexture = TestTexture(renderer, 100);

    return true;
}

bool BoardView::HandleEvent(SDL_Window* window, SDL_Event &ev, Board &board, bool &needsRedraw)
{
    switch (ev.type)
    {
    case SDL_EVENT_KEY_DOWN:
    {
        if (ev.key.scancode == SDL_SCANCODE_ESCAPE)
        {
            // Just reset the turn so we can make the main menu background...
            // ...continue on the same game if there are legal moves to be had
            board.SetTurn(0);
            return true;
        }
    } break;
    case SDL_EVENT_MOUSE_MOTION:
    {
        if (board.IsGameOver()) break;

        m_mouseX = ev.motion.x, m_mouseY = ev.motion.y;

        if (m_showingPromotion)
        {
            Piece* promotingPawn = board.GetPromotingPawn();
            HandlePawnPromotion(promotingPawn, board, needsRedraw);
            break;
        }

        // Check if mouse is dragging a piece
        if (m_isMouseDraggingPiece)
        {
            needsRedraw = true;
            break;
        }
        
        // Update mouse stats
        Piece *piece = GetPieceAt(board, m_mouseX, m_mouseY);
        if (piece && piece != m_lastHoveredPiece && board.GetTurnColor() == piece->GetColor())
        {
            SDL_SetCursor(m_cursorPointer);
            m_lastHoveredPiece = piece;
            m_isMousePointing = true;
        }
        else if (!piece && m_isMousePointing)
        {
            SDL_SetCursor(m_cursorDefault);
            m_lastHoveredPiece = nullptr;
            m_isMousePointing = false;
        }
    } break;
    
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    {
        if (board.IsGameOver()) break;
        
        m_mouseX = ev.button.x, m_mouseY = ev.motion.y;

        // Promotion pawn click handle
        if (m_showingPromotion && m_promotionMouseOver != PIECE_NONE)
        {
            m_showingPromotion = false;
            board.PromotePawn(m_promotionMouseOver);
            if (board.IsGameOver())
            {
                if (!board.GetWinner())
                {
                    ShowStalemateMessage(window);
                }
                else
                {
                    ShowCheckmateMessage(window, board.GetWinner());
                }
                // printf("Game winner: %d\n", board.GetWinner());
            }

            m_isMousePointing = false;
            SDL_SetCursor(m_cursorDefault);

            needsRedraw = true;
            break;
        }
        
        if (ev.button.button == SDL_BUTTON_LEFT && m_isMousePointing && !m_isMouseDraggingPiece)
        {
            Piece* piece = m_lastHoveredPiece;

            float squareSize = m_squareSize;
            float pieceSize = squareSize * 0.8f;

            float squareX = m_offsetX + piece->GetCol() * squareSize;
            float squareY = m_offsetY + piece->GetRow() * squareSize;
            
            float pieceX = squareX + (squareSize - pieceSize) / 2.0f;
            float pieceY = squareY + (squareSize - pieceSize) / 2.0f;

            m_lastHoverOffsetX = m_mouseX - pieceX;
            m_lastHoverOffsetY = m_mouseY - pieceY;

            SDL_SetCursor(m_cursorMove);
            
            // Get possible moves to highlight possible moves while dragging
            m_lastHoveredPiece->CalculatePossibleMoves(board, board.GetTurn());
            
            // Toggle dragging to make the piece move freely
            m_lastHoveredPiece->ToggleDrag();

            m_isMousePointing = false;
            m_isMouseDraggingPiece = true;

            // Play the move sound
            MIX_PlayAudio(m_soundChunk.mixer, m_soundChunk.gMove);
            needsRedraw = true;
        }
    } break;
    
    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
        if (board.IsGameOver()) break;

        m_mouseX = ev.button.x, m_mouseY = ev.button.y;

        // Do nothing if we're waiting for pawn promotion
        if (m_showingPromotion) break;

        if (ev.button.button == SDL_BUTTON_LEFT && m_isMouseDraggingPiece)
        {
            int mouseRow = -1, mouseCol = -1;
            Board::MoveAttemptResult ret = board.MOVE_FAILURE;
            if (PixelToSquare(board, m_mouseX, m_mouseY, mouseRow, mouseCol))
            {
                ret = board.AttemptMove(m_lastHoveredPiece, mouseRow, mouseCol);
                if (ret & board.MOVE_SUCCESS)
                {
                    if (board.IsGameOver())
                    {
                        if (!board.GetWinner())
                        {
                            ShowStalemateMessage(window);
                        }
                        else
                        {
                            ShowCheckmateMessage(window, board.GetWinner());
                        }
                        // printf("Game winner: %d\n", board.GetWinner());
                    }
                    else if (ret & board.MOVE_PROMOTION)
                    {
                        ShowPromotionOptions(board.GetPromotingPawn(), board);
                    }
                }
                else
                {
                    if (ret & board.MOVE_BLACK_KING_EXPOSED)
                    {
                        StartFlashingPiece(board.GetKing(PIECE_COLOR_BLACK));
                    }
                    if (ret & board.MOVE_WHITE_KING_EXPOSED)
                    {
                        StartFlashingPiece(board.GetKing(PIECE_COLOR_WHITE));
                    }
                }
            }
            
            // Picee is no longer free-floating
            m_lastHoveredPiece->ToggleDrag();
            m_lastHoveredPiece = nullptr;
            m_isMouseDraggingPiece = false;
            
            SDL_SetCursor(m_cursorDefault);

            // Play the correct sound
            if (ret & board.MOVE_CAPTURE)
            {
                MIX_PlayAudio(m_soundChunk.mixer, m_soundChunk.gCapture);
            }
            else
            {
                MIX_PlayAudio(m_soundChunk.mixer, m_soundChunk.gMove);
            }
            needsRedraw = true;

            // Maybe check again to see if the mouse is landing on another piece??
        }
    } break;
    }

    return false;
}

void BoardView::Update(Board& board, bool& needsRedraw, float delta)
{
    UpdateFlashing(needsRedraw);

    if (m_showingPromotion)
    {
        constexpr float EPSILON = 0.00001f;
        for (int i = 0; i < 4; ++i)
        {
            Button& btn = m_buttons[i];

            if (!btn.isAnimating)
                continue;

            if (btn.hovered)
            {
                btn.pieceTexScale += (1.2f - btn.pieceTexScale) * 20.0f * delta;
                btn.optTexScale += (1.5f - btn.optTexScale) * 20.0f * delta;
                btn.r += (225 - btn.r) * 20.0f * delta;
                btn.g += (195 - btn.g) * 20.0f * delta;
                btn.b += (145 - btn.b) * 20.0f * delta;
            }
            else
            {
                btn.pieceTexScale += (1.0f - btn.pieceTexScale) * 20.0f * delta;
                btn.optTexScale += (1.0f - btn.optTexScale) * 20.0f * delta;
                btn.r += (255 - btn.r) * 20.0f * delta;
                btn.g += (255 - btn.g) * 20.0f * delta;
                btn.b += (255 - btn.b) * 20.0f * delta;
            }

            btn.isAnimating = fabs(btn.pieceTexScale - 1.2f) > EPSILON && fabs(btn.pieceTexScale - 1.0f) > EPSILON && fabs(btn.optTexScale - 1.5f) > EPSILON && fabs(btn.optTexScale - 1.0f) > EPSILON;
            if (btn.isAnimating)
                needsRedraw = true;
        }
    }
}

bool BoardView::LoadPieceTextures(SDL_Renderer* renderer)
{
    // Do not re-load the textures just in case
    if (s_textures[0][0])
    {
        return true;
    }

    struct TexInfo
    {
        const char* file;
        PieceType type;
        bool white;
    };
    TexInfo files[12]{{"assets/pieces/white-king.png", PIECE_KING, true},
                      {"assets/pieces/white-queen.png", PIECE_QUEEN, true},
                      {"assets/pieces/white-rook.png", PIECE_ROOK, true},
                      {"assets/pieces/white-bishop.png", PIECE_BISHOP, true},
                      {"assets/pieces/white-knight.png", PIECE_KNIGHT, true},
                      {"assets/pieces/white-pawn.png", PIECE_PAWN, true},

                      {"assets/pieces/black-king.png", PIECE_KING, false},
                      {"assets/pieces/black-queen.png", PIECE_QUEEN, false},
                      {"assets/pieces/black-rook.png", PIECE_ROOK, false},
                      {"assets/pieces/black-bishop.png", PIECE_BISHOP, false},
                      {"assets/pieces/black-knight.png", PIECE_KNIGHT, false},
                      {"assets/pieces/black-pawn.png", PIECE_PAWN, false}};

    for (int i = 0; i < 12; ++i)
    {
        TexInfo& f = files[i];
        SDL_Surface* surf = IMG_Load(f.file);
        if (!surf)
        {
            SDL_Log("Failed to load %s: %s", f.file, SDL_GetError());
            return false;
        }
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_DestroySurface(surf);

        if (!tex)
            return false;
        s_textures[f.white ? 0 : 1][f.type] = tex;
    }
    return true;
}

void BoardView::DestroyPieceTextures()
{
    for (int c = 0; c < 2; ++c)
        for (int t = 0; t < 6; ++t)
            if (s_textures[c][t])
            {
                SDL_DestroyTexture(s_textures[c][t]);
                s_textures[c][t] = nullptr;
            }
}

SDL_Texture *BoardView::GetPieceTexture(PieceType type, PieceColor color) const
{
    if (type == PIECE_NONE)
        return nullptr;
    return s_textures[color == PIECE_COLOR_WHITE ? 0 : 1][type];
}

void BoardView::DrawPiece(SDL_Renderer *renderer, Piece* piece) const
{
    SDL_Texture* tex = GetPieceTexture(piece->GetType(), piece->GetColor());
    if (!tex)
        return;

    SDL_FRect dst = GetSquareRect(piece->GetRow(), piece->GetCol());
    SDL_RenderTexture(renderer, tex, nullptr, &dst);
}

void BoardView::DrawPieceAt(SDL_Renderer *renderer, Piece* piece, float x, float y)
{
    SDL_Texture* tex = GetPieceTexture(piece->GetType(), piece->GetColor());
    if (!tex)
        return;

    float pieceSize = m_squareSize * 0.8f;
    SDL_FRect dst = {x, y, pieceSize, pieceSize};
    SDL_RenderTexture(renderer, tex, nullptr, &dst);
}

// For debugging placement of squares
void BoardView::ShowMeSquares(SDL_Renderer* renderer, Board& board)
{
    // Debug
    SDL_FRect mouseRect = {
        m_mouseX - 25,
        m_mouseY - 25,
        50,
        50,
    };

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderRect(renderer, &mouseRect);

    
    float squareSize = m_squareSize;
    float pieceSize = squareSize * 0.8f;

    auto& pieces = board.GetPieces();
    for (Piece* piece : pieces)
    {
        float squareX = m_offsetX + piece->GetCol() * squareSize;
        float squareY = m_offsetY + piece->GetRow() * squareSize;
        
        float pieceX = squareX + (squareSize - pieceSize) / 2.0f;
        float pieceY = squareY + (squareSize - pieceSize) / 2.0f;

        SDL_FRect pieceDebugRect = {
            pieceX,
            pieceY,
            pieceSize,
            pieceSize,
        };
        
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderRect(renderer, &pieceDebugRect);
    }   
}

// Setting a pixel on an SDL_Surface (32-bit RGBA)
void BoardView::SetPixel(SDL_Surface* surface, int x, int y, Uint32 color)
{
    Uint8* pixelAddr = (Uint8*)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32*)pixelAddr = color;
}

// DISABLED: Not used in rendering, kept as an example of interpolation over surface pixels
SDL_Texture* BoardView::TestTexture(SDL_Renderer* renderer, int radius)
{
    int width = radius * 2;
    int height = width;

    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!surface)
    {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        return nullptr;
    }

    SDL_LockSurface(surface);

    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surface->format);

    float r2 = radius*radius;
    for (int y = 0; y < height; ++y)
    {
        float dy = y - radius;
        for (int x = 0; x < width; ++x)
        {
            float dx = x - radius;
            float d2 = dx*dx + dy*dy;
            if (d2 > r2)
            {
                SetPixel(surface, x, y, SDL_MapRGBA(fmt, nullptr, 0, 0, 0, 0));
                continue;
            }
            
            // float dist = std::sqrtf(dx*dx + dy*dy);
            float t = d2 / r2;
            Uint8 darkGray = 120;
            Uint8 lightGray = 200;
            Uint8 intensity = darkGray + (lightGray - darkGray) * (1.0f - t);
            Uint32 color = SDL_MapRGBA(fmt, nullptr, intensity, intensity, intensity, 255);
            SetPixel(surface, x, y, color);
        }
    }

    SDL_UnlockSurface(surface);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    SDL_DestroySurface(surface);

    return texture;
}

SDL_Texture* BoardView::CreateGradientCircle(SDL_Renderer* renderer, int radius, Uint8 edgeColor, Uint8 centerColor)
{
    const int diameter = radius * 2;
    const int radiusSq = radius * radius;
    const float invRadius = 1.0f / radius;
    const float falloff = 2.5f;

    SDL_Surface* surface = SDL_CreateSurface(diameter, diameter, SDL_PIXELFORMAT_RGBA32);
    if (!surface)
    {
        SDL_Log("Failed to create surface: %s", SDL_GetError());
        return nullptr;
    }

    SDL_LockSurface(surface);

    // SDL3: Get pixel format details for mapping colors
    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surface->format);

    for (int y = 0; y < diameter; ++y)
    {
        float dy = y - radius;
        float dySq = dy * dy;
        for (int x = 0; x < diameter; ++x) {
            float dx = x - radius;
            float distSq = dx * dx + dySq;

            if (distSq > radiusSq) {
                // outside circle -> fully transparent
                SetPixel(surface, x, y, SDL_MapRGBA(fmt, nullptr, 0, 0, 0, 0));
                continue;
            }

            float dist = std::sqrt(distSq);
            float t = std::pow(dist * invRadius, falloff);  // 0 = center, 1 = edge

            Uint8 intensity     = (Uint8)(edgeColor + (centerColor - edgeColor) * (1.0f - t));
            Uint8 alpha         = 255;
            Uint32 color        = SDL_MapRGBA(fmt, nullptr, intensity, intensity, intensity, alpha);
            SetPixel(surface, x, y, color);
        }
    }

    SDL_UnlockSurface(surface);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    SDL_DestroySurface(surface);
    return texture;
}

SDL_Rect BoardView::ClipFromFRect(const SDL_FRect& f)
{
    int x1 = (int)floorf(f.x);
    int y1 = (int)floorf(f.y);
    int x2 = (int)ceilf(f.x + f.w);
    int y2 = (int)ceilf(f.y + f.h);
    return SDL_Rect{ x1, y1, x2 - x1, y2 - y1 };
}

void BoardView::Render(SDL_Window* window, SDL_Renderer* renderer, Board& board)
{
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    DrawBoard(window, renderer, board, windowWidth, windowHeight);

    // SDL_Texture* texture = CreateBar(renderer, 128);
    // SDL_FRect rect = { 200.0f, 200.0f, 128.0f, 50.f };
    // SDL_RenderTexture(renderer, texture, nullptr, &rect);

    // SDL_Texture* texture = TestTexture(renderer, 100);
    // SDL_FRect rect = { 200.0f, 200.0f, 50.0f*2.0f, 50.0f*2.0f };

    // float pulse = (sin(SDL_GetTicks() * 0.002f) + 1.0f) * 0.5f; // 0..1
    // Uint8 r = 150 + (Uint8)(105 * pulse); // from 150→255
    // SDL_SetTextureColorMod(m_testTexture, r, r, r);

    // SDL_RenderTexture(renderer, m_testTexture, nullptr, &rect);

    if (m_showingPromotion)
    {
        // Semi-transparent black overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_FRect overlay = { 0, 0, (float)(windowWidth), (float)(windowHeight) };
        SDL_RenderFillRect(renderer, &overlay);

        for (int i = 0; i < 4; ++i)
        {
            Button& btn = m_buttons[i];
            SDL_FRect dst = btn.rect;
            SDL_FRect optTexDst = btn.optRect;
            SDL_FRect pieceTexDst = btn.pieceTexRect;

            // Make sure we update in case window gets resized
            dst = GetSquareRect(btn.row, btn.col);
            
            float baseOptSize = dst.w;
            float baseTexSize = dst.w * 0.8f;

            optTexDst.w = optTexDst.h = baseOptSize;
            pieceTexDst.w = pieceTexDst.h = baseTexSize;

            optTexDst.w *= btn.optTexScale;
            optTexDst.h *= btn.optTexScale;
            optTexDst.x = dst.x + (dst.w - optTexDst.w) / 2;
            optTexDst.y = dst.y + (dst.h - optTexDst.h) / 2;

            pieceTexDst.w *= btn.pieceTexScale;
            pieceTexDst.h *= btn.pieceTexScale;
            pieceTexDst.x = dst.x + (dst.w - pieceTexDst.w) / 2;
            pieceTexDst.y = dst.y + (dst.h - pieceTexDst.h) / 2;

            SDL_Rect clipRect = ClipFromFRect(dst);
            SDL_SetRenderClipRect(renderer, &clipRect);
            SDL_SetTextureColorMod(m_promotionCircle, btn.r, btn.g, btn.b);
            SDL_RenderTexture(renderer, m_promotionCircle, nullptr, &optTexDst);
            SDL_SetRenderClipRect(renderer, nullptr);

            SDL_RenderTexture(renderer, btn.pieceTex, nullptr, &pieceTexDst);
        }
    }
    
    if (m_flashEffect.active && m_flashEffect.visible)
    {
        SDL_FRect flashRect = GetSquareRect(m_flashEffect.piece->GetRow(), m_flashEffect.piece->GetCol());
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 120);
        SDL_RenderFillRect(renderer, &flashRect);
    }

    if (board.IsWhiteKingAttacked())
    {
        Piece* whiteKing = board.GetKing(PIECE_COLOR_WHITE);
        SDL_FRect rect = GetSquareRect(whiteKing->GetRow(), whiteKing->GetCol());
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 120);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &rect);
    }
    else if (board.IsBlackKingAttacked())
    {
        Piece* blackKing = board.GetKing(PIECE_COLOR_BLACK);
        SDL_FRect rect = GetSquareRect(blackKing->GetRow(), blackKing->GetCol());
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 120);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &rect);
    }

    if (m_isMouseDraggingPiece)
    {
        // Highlight the original piece square
        SDL_FRect originRect = GetSquareRect(m_lastHoveredPiece->GetRow(), m_lastHoveredPiece->GetCol());
        
        SDL_SetRenderDrawColor(renderer, 246, 246, 105, 140);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(renderer, &originRect);
        
        // Highlight possible moves
        auto& moves = m_lastHoveredPiece->GetPossibleMoves();
        for (auto& [r, c, isCapture] : moves)
        {
            SDL_FRect sq = GetSquareRect(r, c);

            // If this is a possible capture move
            if (isCapture)
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 120);
                SDL_RenderFillRect(renderer, &sq);
                continue;
            }

            // float dotRadius = sq.w * 0.15f; // 30% diameter
            // float cx = sq.x + sq.w / 2.0f;
            // float cy = sq.y + sq.h / 2.0f;

            // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 90);
            // DrawCircle(renderer, cx, cy, dotRadius);

            float diameter = sq.w * 0.3f;
            sq.x += (sq.w - diameter) / 2;
            sq.y += (sq.h - diameter) / 2;
            sq.w = sq.h = diameter;

            SDL_RenderTexture(renderer, s_circle, nullptr, &sq);
        }

        // Make the piece move with the mouse as the user is dragging
        DrawPieceAt(
            renderer,
            m_lastHoveredPiece,
            m_mouseX - m_lastHoverOffsetX,
            m_mouseY - m_lastHoverOffsetY
        );
    }
}

void BoardView::DrawBoard(SDL_Window *window, SDL_Renderer *renderer, Board& board, float windowWidth, float windowHeight)
{
    float boardSize = m_boardSize = std::min(windowWidth, windowHeight);
    float squareSize = m_squareSize = boardSize / 8.0f;

    float offsetX = m_offsetX = (windowWidth - boardSize) / 2.0f;
    float offsetY = m_offsetY = (windowHeight - boardSize) / 2.0f;

    bool black = false;
    for (int row = 0; row < 8; ++row)
    {
        for (int col = 0; col < 8; ++col)
        {
            SDL_FRect rect = { offsetX + col * squareSize, offsetY + row * squareSize, squareSize, squareSize };
            if ((black = !black))
                SDL_SetRenderDrawColor(renderer, 181, 136, 99, 255);
            else
                SDL_SetRenderDrawColor(renderer, 240, 217, 181, 255);

            SDL_RenderFillRect(renderer, &rect);
        }
        black = !black;
    }

    DrawPieces(renderer, board, squareSize, offsetX, offsetY);
}

void BoardView::DrawPieces(SDL_Renderer *renderer, Board& board, float squareSize, float offsetX, float offsetY)
{
    for (Piece* piece : board.GetPieces())
    {
        if (piece->IsDead()) continue;
        if (piece->IsDragged()) continue;
        DrawPiece(renderer, piece);
    }
}

void BoardView::ShowCheckmateMessage(SDL_Window* window, PieceColor winner)
{
    const char* title = "Checkmate";
    const char* message = (winner == PIECE_COLOR_WHITE) ?
        "White wins by checkmate! Press ESC to go back to main menu." :
        "Black wins by checkmate! Press ESC to go back to main menu.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, window);
}

void BoardView::ShowStalemateMessage(SDL_Window* window)
{
    const char* title = "Stalemate";
    const char* message = "Stalemate! Press ESC to go back to main menu.";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, window);
}

Piece* BoardView::GetPieceAt(Board& board, float x, float y) const
{
    for (Piece* piece : board.GetPieces())
    {
        if (piece->IsDead()) continue;
        SDL_FRect rect = GetSquareRect(piece->GetRow(), piece->GetCol());
        float rectW = rect.w * 0.9f;
        float rectH = rect.h * 0.9f;
        float offsetX = (rect.w - rectW) / 2;
        float offsetY = (rect.h - rectH) / 2;
        // SDL_FRect rect = piece->GetBoundingBox(m_squareSize, m_offsetX, m_offsetY);
        if (x >= rect.x + offsetX && x <= rect.x + rectW && y >= offsetY + rect.y && y <= rect.y + rectH)
            return piece;
    }
    return nullptr;
}

bool BoardView::PixelToSquare(Board& board, float x, float y, int& outRow, int& outCol) const
{
    float sx = x - m_offsetX;
    float sy = y - m_offsetY;
    if (sx < 0 || sy < 0) return false;

    int col = (int)(sx / m_squareSize);
    int row = (int)(sy / m_squareSize);

    if (!board.IsValidSquare(row, col)) return false;

    outRow = row;
    outCol = col;
    return true;
}

void BoardView::ShowPromotionOptions(Piece* promotingPawn, Board& board)
{
    m_showingPromotion = true;

    PieceColor pawnColor = promotingPawn->GetColor();
    int col = promotingPawn->GetCol();
    int row = promotingPawn->GetRow();

    const float dir = (pawnColor == board.GetBottomPlayer()) ? 1.0f : -1.0f;

    for (int i = 0; i < 4; ++i)
    {
        int optRow = (int)(row + dir * i);
        int optCol = col;

        SDL_FRect rect = GetSquareRect(optRow, optCol);
        SDL_Texture* pieceTexture = GetPieceTexture((PieceType)(i + 1), pawnColor);

        m_buttons[i].row = optRow;
        m_buttons[i].col = optCol;

        m_buttons[i].rect = m_buttons[i].optRect = rect;
        m_buttons[i].pieceTex = pieceTexture;
        m_buttons[i].pieceTexRect = rect;

        m_buttons[i].pieceTexRect.w *= 0.8f;
        m_buttons[i].pieceTexRect.h *= 0.8f;

        const float offsets = m_buttons[i].rect.w - m_buttons[i].pieceTexRect.w;
        m_buttons[i].pieceTexRect.x = m_buttons[i].rect.x + offsets / 2;
        m_buttons[i].pieceTexRect.y = m_buttons[i].rect.y + offsets / 2;
    }
}

void BoardView::HandlePawnPromotion(Piece *pawn, Board &board, bool &needsRedraw)
{
    bool isPointingAtOpt = false;
    PieceColor pawnColor = pawn->GetColor();
    int pawnRow = pawn->GetRow();
    int pawnCol = pawn->GetCol();

    // Match the direction used when drawing promotion options
    int selectionRowDir = (pawnColor == board.GetBottomPlayer()) ? +1 : -1;

    // Check each promotion option square: Queen (1) -> Knight (4)
    for (int i = 0; i < 4; ++i)
    {
        int optRow = pawnRow + selectionRowDir * i;
        int optCol = pawnCol;

        SDL_FRect rect = GetSquareRect(optRow, optCol);

        // Quick reject: mouse must be inside the square first
        if (m_mouseX < rect.x || m_mouseX > rect.x + rect.w ||
            m_mouseY < rect.y || m_mouseY > rect.y + rect.h)
        {
            if (m_buttons[i].hovered)
                m_buttons[i].isAnimating = true;
            m_buttons[i].hovered = false;
            continue;
        }

        // Now check if the mouse point is inside the centered circle (radius = half square)
        float cx = rect.x + rect.w * 0.5f;
        float cy = rect.y + rect.h * 0.5f;
        float radius = rect.w * 0.5f;

        if (IsPointInCircle(m_mouseX, m_mouseY, cx, cy, radius))
        {
            if (!m_buttons[i].hovered)
                m_buttons[i].isAnimating = true;
            m_buttons[i].hovered = true;
            m_promotionMouseOver = (PieceType)(i + 1);
            isPointingAtOpt = true;
        }
        else
        {
            if (m_buttons[i].hovered)
                m_buttons[i].isAnimating = true;
            m_buttons[i].hovered = false;
        }
    }
    if (isPointingAtOpt)
    {
        if (!m_isMousePointing)
        {
            SDL_SetCursor(m_cursorPointer);
            m_isMousePointing = true;
        }
    }
    else
    {
        m_promotionMouseOver = PIECE_NONE;
        if (m_isMousePointing)
        {
            SDL_SetCursor(m_cursorDefault);
            m_isMousePointing = false;
        }
    }
}

void BoardView::StartFlashingPiece(Piece* piece)
{
    m_flashEffect.piece = piece;
    m_flashEffect.startTime = SDL_GetTicks();
    m_flashEffect.duration = 2000;
    m_flashEffect.visible = false;
    m_flashEffect.active = true;
}

void BoardView::UpdateFlashing(bool& needsRedraw)
{
    if (m_flashEffect.active)
    {
        Uint32 elapsed = SDL_GetTicks() - m_flashEffect.startTime;
        if (elapsed >= m_flashEffect.duration)
        {
            m_flashEffect.piece = nullptr;
            m_flashEffect.visible = false;
            m_flashEffect.active = false;
            needsRedraw = true;
            return;
        }
        
        bool newVisible = ((elapsed / 500) % 2 == 0);
        if (newVisible != m_flashEffect.visible)
        {
            m_flashEffect.visible = newVisible;
            needsRedraw = true;
        }
    }
}
