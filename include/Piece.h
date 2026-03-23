#pragma once
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include <tuple>
#include <array>

using PIECE_ROW = int;
using PIECE_COL = int;
using PIECE_CAPTURE = bool;

enum PieceType
{
    PIECE_KING,
    PIECE_QUEEN,
    PIECE_ROOK,
    PIECE_BISHOP,
    PIECE_KNIGHT,
    PIECE_PAWN,
    PIECE_NONE // for empty/dead pieces
};

enum PieceColor
{
    PIECE_COLOR_NONE,
    PIECE_COLOR_WHITE,
    PIECE_COLOR_BLACK,
};

class Board;

class Piece
{
public:
    // Piece() :
    //     m_type(PIECE_NONE),
    //     m_row(-1),
    //     m_col(-1),
    //     m_prevRow(0),
    //     m_prevCol(0),
    //     m_moves(0),
    //     m_isEnPassantableTurn(0),
    //     m_color(PIECE_COLOR_WHITE),
    //     m_isDragged(false)
    // {
    //     m_cachedMoves.reserve(27);
    // }

    Piece();
    Piece(PieceType type, int col, int row, PieceColor color);

    // --- Gameplay ---
    const std::vector<std::tuple<PIECE_ROW, PIECE_COL, PIECE_CAPTURE>>& GetPossibleMoves() const { return m_cachedMoves; }
    const std::vector<std::tuple<PIECE_ROW, PIECE_COL, PIECE_CAPTURE>>& CalculatePossibleMoves(const Board& board, int currentTurn) const;
    void Reset();

    void Move(int row, int col);
    void UndoLastMove();

    // --- Getters ---
    PieceType GetType() const { return m_type; }
    void SetType(PieceType type) { m_type = type; }

    int GetCol() const { return m_col; }
    int GetRow() const { return m_row; }
    int GetMoves() const { return m_moves; }
    PieceColor GetColor() const { return m_color; }
    bool IsWhite() const { return m_color == PIECE_COLOR_WHITE; }
    bool IsDragged() const { return m_isDragged; }
    bool IsDead() const { return m_type == PIECE_NONE || (m_row == -1 && m_col == -1); }

    void ToggleDrag() { m_isDragged = !m_isDragged; }

    PieceColor GetOpponentColor() const { return m_color == PIECE_COLOR_WHITE ? PIECE_COLOR_BLACK : PIECE_COLOR_WHITE; }

    void SetEnpassantableTurn(int turn) { m_isEnPassantableTurn = turn; }
    int GetEnpassantableTurn() const { return m_isEnPassantableTurn; }

private:
    void CalculateMoves(const Board &board, int currentTurn) const;

    mutable std::vector<std::tuple<PIECE_ROW, PIECE_COL, PIECE_CAPTURE>> m_cachedMoves;

    PieceType m_type;
    int m_row, m_col;
    int m_prevRow, m_prevCol;
    int m_moves;
    int m_isEnPassantableTurn;
    PieceColor m_color;
    bool m_isDragged;
};
