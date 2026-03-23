#include "Board.h"
#include <algorithm>
#include <iostream>

Board::Board() :
    m_blackKing(nullptr), m_isBlackKingAttacked(false),
    m_whiteKing(nullptr), m_isWhiteKingAttacked(false),
    m_turn(1),
    m_turnColor(PIECE_COLOR_WHITE),
    m_gameOver(false), m_gameWinner(PIECE_COLOR_NONE),
    m_promotingPawn(nullptr),
    m_gen(std::chrono::system_clock::now().time_since_epoch().count())
{
}

Piece* Board::GetPieceAt(int row, int col) const
{
    for (Piece* piece : m_pieces)
    {
        if (!piece->IsDead() && piece->GetRow() == row && piece->GetCol() == col)
            return piece;
    }
    return nullptr;
}

bool Board::CheckKingChecked(PieceColor kingColor, int currentTurn)
{
    Piece* king = GetKing(kingColor);
    if (!king || king->IsDead()) return false;

    int kingRow = king->GetRow();
    int kingCol = king->GetCol();

    for (Piece* piece : m_pieces)
    {
        if (piece->GetColor() == kingColor) continue;
        if (piece->IsDead()) continue;

        auto& possibleMoves = piece->CalculatePossibleMoves(*this, currentTurn);
        for (auto& [r, c, isCapture] : possibleMoves)
        {
            if (isCapture && r == kingRow && c == kingCol)
                return true;
        }
    }
    return false;
}

bool Board::HasLegalMoves(PieceColor color, int currentTurn)
{
    for (Piece* piece : m_pieces)
    {
        if (piece->GetColor() != color) continue;
        if (piece->IsDead()) continue;

        auto& moves = piece->CalculatePossibleMoves(*this, currentTurn);
        for (auto& [r, c, isCapture] : moves)
        {
            Piece* captured = nullptr;
            if (isCapture)
            {
                captured = GetPieceAt(r, c);
                if (!captured)
                {
                    int pawnEnemyRow = color == m_bottomPlayerColor ? r + 1 : r - 1;
                    captured = GetPieceAt(pawnEnemyRow, c);
                }
                captured->Move(-1, -1);
            }
            piece->Move(r, c);

            bool kingChecked = CheckKingChecked(color, currentTurn);

            piece->UndoLastMove();
            if (isCapture) captured->UndoLastMove();

            if (!kingChecked) return true;
        }
    }
    return false;
}

bool Board::HasLegalMoves(PieceColor color, int currentTurn, const std::vector<std::tuple<PIECE_ROW, PIECE_COL, PIECE_CAPTURE>>& moves)
{
    for (Piece* piece : m_pieces)
    {
        if (piece->GetColor() != color) continue;
        if (piece->IsDead()) continue;

        for (auto& [r, c, isCapture] : moves)
        {
            Piece* captured = nullptr;
            if (isCapture)
            {
                captured = GetPieceAt(r, c);
                if (!captured)
                {
                    int pawnEnemyRow = color == m_bottomPlayerColor ? r + 1 : r - 1;
                    captured = GetPieceAt(pawnEnemyRow, c);
                }
                captured->Move(-1, -1);
            }
            piece->Move(r, c);

            bool kingChecked = CheckKingChecked(color, currentTurn);

            piece->UndoLastMove();
            if (isCapture) captured->UndoLastMove();

            if (!kingChecked) return true;
        }
    }
    return false;
}

void Board::PromotePawn(PieceType type)
{
    if (m_promotingPawn)
    {
        m_promotingPawn->SetType(type);

        PieceColor opponentColor = GetOpponentColor(m_promotingPawn->GetColor());
        if (CheckKingChecked(opponentColor, m_turn))
        {
            SetKingAttacked(opponentColor, true);
        }
        if (!HasLegalMoves(opponentColor, m_turn))
        {
            m_gameOver = true;
            m_gameWinner = IsKingAttacked(opponentColor) ? m_promotingPawn->GetColor() : PIECE_COLOR_NONE;
        }
        m_promotingPawn = nullptr;
    }
}

Board::MoveAttemptResult Board::AttemptMove(Piece* piece, int row, int col)
{
    unsigned int result = MOVE_FAILURE;
    auto& possibleMoves = piece->GetPossibleMoves();
    for (auto& [possibleRow, possibleCol, isCapture] : possibleMoves)
    {
        if (row == possibleRow && col == possibleCol)
        {
            int pieceCurRow = piece->GetRow();
            int pieceCurCol = piece->GetCol();
            PieceColor pieceColor = piece->GetColor();
            PieceColor opponentColor = pieceColor == PIECE_COLOR_BLACK ? PIECE_COLOR_WHITE : PIECE_COLOR_BLACK;
            PieceType pieceType = piece->GetType();

            Piece* enemyPiece = nullptr;
            if (isCapture)
            {
                result |= MOVE_CAPTURE;
                enemyPiece = GetPieceAt(row, col);
                if (!enemyPiece)
                {
                    // isCapture and enemyPiece doesn't exist = en passant
                    int pawnEnemyRow = pieceColor == m_bottomPlayerColor ? row + 1 : row - 1;
                    enemyPiece = GetPieceAt(pawnEnemyRow, col);
                }
                enemyPiece->Move(-1, -1);
            }
            
            SpecialCase specialCase = GAME_NORMAL;
            switch (pieceType)
            {
            case PIECE_KING:
                if (col - pieceCurCol == 2)
                {
                    specialCase = GAME_CASTLING;
                }
                break;
            case PIECE_PAWN:
                if (row - pieceCurRow == 2 || row - pieceCurRow == -2)
                {
                    specialCase = GAME_ENPASSANTABLE;
                }
                else if ((pieceColor == m_bottomPlayerColor && row == 0) || (pieceColor != m_bottomPlayerColor && row == 7))
                {
                    specialCase = GAME_PROMOTION;
                }
                break;
            default: break;
            }
            
            piece->Move(row, col);
            pieceCurRow = row;
            pieceCurCol = col;

            // Check if this player is leaving their king exposed
            if (CheckKingChecked(pieceColor, m_turn))
            {
                // Undo
                piece->UndoLastMove();
                if (isCapture) enemyPiece->UndoLastMove();
                if (pieceColor == PIECE_COLOR_WHITE) result |= MOVE_WHITE_KING_EXPOSED;
                else result |= MOVE_BLACK_KING_EXPOSED;
                return (Board::MoveAttemptResult)result;
            }
            else
            {
                if (pieceColor == PIECE_COLOR_WHITE) m_isWhiteKingAttacked = false; else m_isBlackKingAttacked = false;
            }
            
            // Handle special case
            switch (specialCase)
            {
            case GAME_CASTLING:
            {
                Piece* rookPiece = GetPieceAt(row, col + 1);
                rookPiece->Move(row, col - 1);
            } break;
            case GAME_ENPASSANTABLE:
            {
                piece->SetEnpassantableTurn(m_turn + 1);
                break;
            }
            case GAME_PROMOTION:
            {
                m_promotingPawn = piece;
                result |= MOVE_PROMOTION;
                break;
            }
            case GAME_NORMAL: break;
            }
            
            // Check if opponent's king is checked
            if (CheckKingChecked(opponentColor, m_turn))
            {
                if (opponentColor == PIECE_COLOR_WHITE)
                    m_isWhiteKingAttacked = true;
                else
                    m_isBlackKingAttacked = true;
            }
            
            // Check if opponent has any possible legal moves before giving the turn back to them
            if (!HasLegalMoves(opponentColor, m_turn))
            {
                m_gameOver = true;

                if (m_isWhiteKingAttacked) m_gameWinner = PIECE_COLOR_BLACK;
                else if (m_isBlackKingAttacked) m_gameWinner = PIECE_COLOR_WHITE;
                else m_gameWinner = PIECE_COLOR_NONE;
                result |= MOVE_SUCCESS;
                return (Board::MoveAttemptResult)MOVE_SUCCESS;
            }
            
            m_turn++;
            m_turnColor = opponentColor;
            result |= MOVE_SUCCESS;
            return (Board::MoveAttemptResult)result;
        }
    }
    
    return MOVE_FAILURE;
}

bool Board::AttemptRandomMove()
{
    if (m_gameOver)
    {
        return false;
    }

    std::uniform_int_distribution<> pieceDistrib(0, 15);
    for (;;)
    {
        Piece* randomPiece;

        int randomIdx = pieceDistrib(m_gen);
        if (m_turnColor == PIECE_COLOR_WHITE)
        {
            randomPiece = &m_whitePieces[randomIdx];
        }
        else
        {
            randomPiece = &m_blackPieces[randomIdx];
        }

        auto& possibleMoves = randomPiece->CalculatePossibleMoves(*this, m_turn);
        if (possibleMoves.empty())
        {
            continue;
        }
        
        std::uniform_int_distribution<> moveDistrib(0, possibleMoves.size() - 1);
        randomIdx = moveDistrib(m_gen);
        auto& move = possibleMoves[randomIdx];
        Board::MoveAttemptResult res = AttemptMove(randomPiece, std::get<0>(move), std::get<1>(move));
        if (!(res & MOVE_SUCCESS)) continue;
        if (res & MOVE_PROMOTION)
        {
            PromotePawn(PIECE_QUEEN);
        }
        if (m_gameOver)
        {
            return false;
        }
        return true;
    }

    return false;
}

void Board::Init(SDL_Renderer *renderer, PieceColor bottomColor)
{
    m_bottomPlayerColor = bottomColor;
    InitPieces(renderer, bottomColor);
}

void Board::InitPieces(SDL_Renderer *renderer, PieceColor bottomColor)
{
    m_pieces.clear();

    Piece* bottomPieces     = nullptr;
    Piece* topPieces        = nullptr;
    Piece** bottomKing       = nullptr;
    Piece** topKing          = nullptr;
    if (bottomColor == PIECE_COLOR_BLACK)
    {
        bottomPieces = m_blackPieces;
        topPieces = m_whitePieces;
        bottomKing = &m_blackKing;
        topKing = &m_whiteKing;
    }
    else
    {
        bottomPieces = m_whitePieces;
        topPieces = m_blackPieces;
        bottomKing = &m_whiteKing;
        topKing = &m_blackKing;
    }

    // Bottom pieces
    int idx = 0;
    bottomPieces[idx++] = Piece(PIECE_ROOK,   0, 7, bottomColor);
    bottomPieces[idx++] = Piece(PIECE_KNIGHT, 1, 7, bottomColor);
    bottomPieces[idx++] = Piece(PIECE_BISHOP, 2, 7, bottomColor);
    bottomPieces[idx++] = Piece(PIECE_QUEEN,  3, 7, bottomColor);
    bottomPieces[idx++] = Piece(PIECE_KING,   4, 7, bottomColor);
    *bottomKing = &bottomPieces[4];
    bottomPieces[idx++] = Piece(PIECE_BISHOP, 5, 7, bottomColor);
    bottomPieces[idx++] = Piece(PIECE_KNIGHT, 6, 7, bottomColor);
    bottomPieces[idx++] = Piece(PIECE_ROOK,   7, 7, bottomColor);
    for (int i = 0; i < 8; i++)
        bottomPieces[idx++] = Piece(PIECE_PAWN, i, 6, bottomColor);

    // Top pieces
    idx = 0;
    topPieces[idx++] = Piece(PIECE_ROOK,   0, 0, GetOpponentColor(bottomColor));
    topPieces[idx++] = Piece(PIECE_KNIGHT, 1, 0, GetOpponentColor(bottomColor));
    topPieces[idx++] = Piece(PIECE_BISHOP, 2, 0, GetOpponentColor(bottomColor));
    topPieces[idx++] = Piece(PIECE_QUEEN,  3, 0, GetOpponentColor(bottomColor));
    topPieces[idx++] = Piece(PIECE_KING,   4, 0, GetOpponentColor(bottomColor));
    *topKing = &topPieces[4];
    topPieces[idx++] = Piece(PIECE_BISHOP, 5, 0, GetOpponentColor(bottomColor));
    topPieces[idx++] = Piece(PIECE_KNIGHT, 6, 0, GetOpponentColor(bottomColor));
    topPieces[idx++] = Piece(PIECE_ROOK,   7, 0, GetOpponentColor(bottomColor));
    for (int i = 0; i < 8; i++)
        topPieces[idx++] = Piece(PIECE_PAWN, i, 1, GetOpponentColor(bottomColor));

    // Fill m_pieces with pointers
    for (auto& p : m_whitePieces) m_pieces.push_back(&p);
    for (auto& p : m_blackPieces) m_pieces.push_back(&p);
}

void Board::Reset(bool resetPawns)
{
    // m_blackKing = nullptr;
    m_isBlackKingAttacked = false;
    // m_whiteKing = nullptr;
    m_isWhiteKingAttacked = false;

    Piece *bottomPieces, *topPieces;
    if (m_bottomPlayerColor == PIECE_COLOR_BLACK)
    {
        bottomPieces = m_blackPieces;
        topPieces = m_whitePieces;
    }
    else
    {
        bottomPieces = m_whitePieces;
        topPieces = m_blackPieces;
    }

    // White pieces are ordered first
    // Rook -> Knight -> Bishop -> Queen -> King -> Bishop -> Knight -> Rook -> Pawns
    for (int i = 0; i < 8; ++i)
    {
        (bottomPieces + i)->Move(7, i);
        (bottomPieces + i)->Reset();

        (bottomPieces + 1*8+i)->Move(6, i);
        if (resetPawns) (bottomPieces + 1*8+i)->SetType(PIECE_PAWN);
        (bottomPieces + 1*8+i)->Reset();


        (topPieces + i)->Move(0, i);
        (topPieces + i)->Reset();

        (topPieces + 1*8+i)->Move(1, i);
        if (resetPawns) (topPieces + 1*8+i)->SetType(PIECE_PAWN);
        (topPieces + 1*8+i)->Reset();
    }

    m_turn = 1;
    m_turnColor = m_bottomPlayerColor;
    m_gameOver = false;
    m_gameWinner = PIECE_COLOR_NONE;
    m_promotingPawn = nullptr;
}
