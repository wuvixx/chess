#include "Piece.h"
#include "Board.h"
#include <algorithm>

Piece::Piece()
{
}

Piece::Piece(PieceType type, int col, int row, PieceColor color) : m_type(type),
                                                                   m_row(row),
                                                                   m_col(col),
                                                                   m_prevRow(0),
                                                                   m_prevCol(0),
                                                                   m_moves(0),
                                                                   m_isEnPassantableTurn(0),
                                                                   m_color(color),
                                                                   m_isDragged(false)
{
    m_cachedMoves.reserve(27);
}

void Piece::CalculateMoves(const Board& board, int currentTurn) const
{
    switch (m_type)
    {
    case PIECE_PAWN:
    {
        int forwardRow = m_row + (m_color == board.GetBottomPlayer() ? -1 : +1);
        if (board.IsValidSquare(forwardRow, m_col) && !board.GetPieceAt(forwardRow, m_col))
        {
            m_cachedMoves.push_back({forwardRow, m_col, false});

            // Check if the pawn can make two steps
            if (!m_moves)
            {
                int secondForwardRow = forwardRow + (m_color == board.GetBottomPlayer() ? -1 : +1);
                if (board.IsValidSquare(secondForwardRow, m_col) && !board.GetPieceAt(secondForwardRow, m_col))
                {
                    m_cachedMoves.push_back({secondForwardRow, m_col, false});
                }
            }
        }

        // Check if the pawn can eat enemy piece
        int pawnColMoves[2] = {1, -1};
        for (int i = 0; i < 2; ++i)
        {
            if (board.IsValidSquare(forwardRow, m_col + pawnColMoves[i]))
            {
                Piece* piece = board.GetPieceAt(forwardRow, m_col + pawnColMoves[i]);
                if (piece && m_color != piece->GetColor())
                {
                    m_cachedMoves.push_back({forwardRow, m_col + pawnColMoves[i], true});
                }
            }
        }

        // Check if pawn can perform en passant
        int colToCheck[2] = {1, -1};
        for (int i = 0; i < 2; ++i)
        {
            if (!board.IsValidSquare(m_row, m_col + colToCheck[i]))
                continue;
            Piece* piece = board.GetPieceAt(m_row, m_col + colToCheck[i]);
            if (piece && piece->GetType() == PIECE_PAWN && piece->GetEnpassantableTurn() == currentTurn)
            {
                int row = m_color == board.GetBottomPlayer() ? m_row - 1 : m_row + 1;
                m_cachedMoves.push_back({row, m_col + colToCheck[i], true});
            }
        }
    } break;

    case PIECE_ROOK:
    {
        for (int i = m_row + 1; i < 8; ++i)
        {
            Piece* piece = board.GetPieceAt(i, m_col);
            if (piece)
            {
                if (piece->GetColor() != GetColor())
                {
                    m_cachedMoves.push_back({i, m_col, true});
                }
                break;
            }
            m_cachedMoves.push_back({i, m_col, false});
        }

        for (int i = m_row - 1; i >= 0; --i)
        {
            Piece* piece = board.GetPieceAt(i, m_col);
            if (piece)
            {
                if (piece->GetColor() != GetColor())
                {
                    m_cachedMoves.push_back({i, m_col, true});
                }
                break;
            }
            m_cachedMoves.push_back({i, m_col, false});
        }

        for (int i = m_col + 1; i < 8; ++i)
        {
            Piece* piece = board.GetPieceAt(m_row, i);
            if (piece)
            {
                if (piece->GetColor() != GetColor())
                {
                    m_cachedMoves.push_back({m_row, i, true});
                }
                break;
            }
            m_cachedMoves.push_back({m_row, i, false});
        }

        for (int i = m_col - 1; i >= 0; --i)
        {
            Piece* piece = board.GetPieceAt(m_row, i);
            if (piece)
            {
                if (piece->GetColor() != GetColor())
                {
                    m_cachedMoves.push_back({m_row, i, true});
                }
                break;
            }
            m_cachedMoves.push_back({m_row, i, false});
        }
    } break;

    case PIECE_BISHOP:
    {
        int bishopMoves[4][2] = {
            {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
        for (int i = 0; i < 4; ++i)
        {
            int row = m_row, col = m_col;
            for (;;)
            {
                row += bishopMoves[i][0];
                col += bishopMoves[i][1];

                if (!board.IsValidSquare(row, col))
                {
                    break;
                }

                Piece* piece = board.GetPieceAt(row, col);
                if (piece)
                {
                    if (piece->GetColor() != GetColor())
                    {
                        // @TODO: Handle opponent piece
                        // Different highlight color, perhaps?
                        m_cachedMoves.push_back({row, col, true});
                    }
                    break;
                }

                m_cachedMoves.push_back({row, col, false});
            }
        }
    } break;

    case PIECE_KNIGHT:
    {
        int knightMoves[8][2] = {
            {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
        for (int i = 0; i < 8; ++i)
        {
            int row = m_row + knightMoves[i][0];
            int col = m_col + knightMoves[i][1];

            Piece* piece = board.GetPieceAt(row, col);
            if (piece)
            {
                if (piece->GetColor() != GetColor())
                {
                    // @TODO: Handle opponent piece
                    // Different highlight color, perhaps?
                    m_cachedMoves.push_back({row, col, true});
                }
                continue;
                ;
            }

            if (board.IsValidSquare(row, col))
            {
                m_cachedMoves.push_back({row, col, false});
            }
        }
    } break;

    case PIECE_QUEEN:
    {
        int queenMoves[8][2] = {
            {0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};

        for (int i = 0; i < 8; ++i)
        {
            int row = m_row, col = m_col;
            for (;;)
            {
                row += queenMoves[i][0];
                col += queenMoves[i][1];

                if (!board.IsValidSquare(row, col))
                {
                    break;
                }

                Piece* piece = board.GetPieceAt(row, col);
                if (piece)
                {
                    if (piece->GetColor() != GetColor())
                    {
                        // @TODO: Handle opponent piece
                        // Different highlight color, perhaps?
                        m_cachedMoves.push_back({row, col, true});
                    }
                    break;
                }

                m_cachedMoves.push_back({row, col, false});
            }
        }
    } break;

    case PIECE_KING:
    {
        int kingMoves[8][2] = {
            {0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, -1}, {-1, 1}, {1, 1}, {-1, -1}};
        for (int i = 0; i < 8; ++i)
        {
            int row = m_row + kingMoves[i][0];
            int col = m_col + kingMoves[i][1];

            Piece* piece = board.GetPieceAt(row, col);
            if (piece)
            {
                if (m_color != piece->GetColor())
                {
                    // @TODO: Handle opponent piece
                    // Different highlight color, perhaps?
                    m_cachedMoves.push_back({row, col, true});
                }
                continue;
            }

            if (board.IsValidSquare(row, col))
            {
                m_cachedMoves.push_back({row, col, false});
            }
        }

        // Castling
        if (!m_moves && !board.GetPieceAt(m_row, m_col + 1) && !board.GetPieceAt(m_row, m_col + 2))
        {
            Piece* castlingRook = nullptr;
            if (m_color == board.GetBottomPlayer())
            {
                castlingRook = board.GetPieceAt(7, 7);
            }
            else
            {
                castlingRook = board.GetPieceAt(0, 7);
            }

            if (castlingRook && !castlingRook->GetMoves())
            {
                m_cachedMoves.push_back({castlingRook->GetRow(), castlingRook->GetCol() - 1, false});
            }
        }
    } break;
    
    case PIECE_NONE: break;
    }
}

std::vector<std::tuple<int, int, bool>> const& Piece::CalculatePossibleMoves(const Board &board, int currentTurn) const
{
    m_cachedMoves.clear();
    if (IsDead())
    {
        return m_cachedMoves;
    }

    CalculateMoves(board, currentTurn);
    return m_cachedMoves;
}

void Piece::Reset()
{
    m_cachedMoves.clear();
    m_moves = 0;
    m_isEnPassantableTurn = 0;
}

void Piece::Move(int row, int col)
{
    m_prevRow = m_row;
    m_prevCol = m_col;
    m_row = row;
    m_col = col;
    m_moves++;
}

void Piece::UndoLastMove()
{
    if (m_moves)
    {
        m_row = m_prevRow;
        m_col = m_prevCol;
        m_moves--;
    }
}
