#pragma once
#include "Piece.h"
#include <SDL3/SDL.h>
#include <vector>
#include <chrono>
#include <random>

class Board
{
    public:
    Board();
    void Reset(bool resetPawns);
    
    enum MoveAttemptResult
    {
        MOVE_FAILURE = 0,
        MOVE_SUCCESS = 1 << 0,
        MOVE_WHITE_KING_EXPOSED = 1 << 1,
        MOVE_BLACK_KING_EXPOSED = 1 << 2,
        MOVE_CAPTURE = 1 << 3,
        MOVE_PROMOTION = 1 << 4,
    };

    Piece* GetPieceAt(int row, int col) const;
    bool IsValidSquare(int row, int col) const { return row >= 0 && row < 8 && col >= 0 && col < 8; }
    
    // Returns whether we should update view or not
    MoveAttemptResult AttemptMove(Piece* piece, int row, int col);

    bool AttemptRandomMove();

    bool CheckKingChecked(PieceColor kingColor, int currentTurn);
    // bool IsCheckmate(PieceColor kingColor, int currentTurn);

    bool HasLegalMoves(PieceColor color, int currentTurn);
    bool HasLegalMoves(PieceColor color, int currentTurn, const std::vector<std::tuple<PIECE_ROW, PIECE_COL, PIECE_CAPTURE>>& moves);

    void PromotePawn(PieceType type);

public:
    // Getters/Setters
    std::vector<Piece*>& GetPieces() const { return m_pieces; }
    Piece* GetKing(PieceColor color) const { return (color == PIECE_COLOR_WHITE) ? m_whiteKing : m_blackKing; }
    Piece* GetPromotingPawn() const { return m_promotingPawn; }
    PieceColor GetTurnColor() const { return m_turnColor; }
    int GetTurn() const { return m_turn; }
    void SetTurn(int turn) const { m_turn = turn; }
    bool IsGameOver() const { return m_gameOver; }
    PieceColor GetWinner() const { return m_gameWinner; }
    bool IsWhiteKingAttacked() const { return m_isWhiteKingAttacked; }
    bool IsBlackKingAttacked() const { return m_isBlackKingAttacked; }
    bool IsKingAttacked(PieceColor color) const { return color == PIECE_COLOR_WHITE ? IsWhiteKingAttacked() : IsBlackKingAttacked(); }
    void SetWhiteKingAttacked(bool attacked) const { m_isWhiteKingAttacked = attacked; }
    void SetBlackKingAttacked(bool attacked) const { m_isBlackKingAttacked = attacked; }
    void SetKingAttacked(PieceColor color, bool attacked) const { return color == PIECE_COLOR_WHITE ? SetWhiteKingAttacked(attacked) : SetBlackKingAttacked(attacked); }
    PieceColor GetBottomPlayer() const { return m_bottomPlayerColor; }
    PieceColor GetOpponentColor(PieceColor color) const { return color == PIECE_COLOR_WHITE ? PIECE_COLOR_BLACK : PIECE_COLOR_WHITE; }

public:
    void Init(SDL_Renderer* renderer, PieceColor bottomColor);
    void InitPieces(SDL_Renderer* renderer, PieceColor bottomColor);

private:
    Piece* m_blackKing;
    mutable bool m_isBlackKingAttacked;
    Piece* m_whiteKing;
    mutable bool m_isWhiteKingAttacked;

    mutable std::vector<Piece*> m_pieces;

    // Now we allocate pieces on the stack (arrays), not on the heap
    Piece m_whitePieces[16];
    Piece m_blackPieces[16];
    
    PieceColor m_bottomPlayerColor;
    
    mutable int m_turn;
    PieceColor m_turnColor;
    bool m_gameOver;
    PieceColor m_gameWinner;
    
    // Pawn getting promoted
    Piece* m_promotingPawn;
    
    enum SpecialCase
    {
        GAME_NORMAL,
        GAME_CASTLING,
        GAME_ENPASSANTABLE,
        GAME_PROMOTION,
    };

    // Generator
    std::mt19937 m_gen;
};
