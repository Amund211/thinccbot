#ifndef PIECES_H_INCLUDED
#define PIECES_H_INCLUDED

#include <cstdint>
#include <string>

enum Color : uint8_t
{
	BLACK = 0,
	WHITE = 8
};

enum PieceType : uint8_t
{
	NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, UNKNOWN
};

typedef uint8_t Piece;

const std::string pieceToUnicode[16] = {
	" ",
	"♙",
	"♘",
	"♗",
	"♖",
	"♕",
	"♔",
	"?",
	"?",
	"♟︎",
	"♞",
	"♝",
	"♜",
	"♛",
	"♚",
	"?"
};

constexpr char pieceToSymbol[16] = {
	' ',
	'p',
	'n',
	'b',
	'r',
	'q',
	'k',
	'?',
	'?',
	'P',
	'N',
	'B',
	'R',
	'Q',
	'K',
	'?'
};

Piece symbolToPiece(char symbol);

inline PieceType pieceType(Piece piece)
{
	// Lower 3 bits
	return static_cast<PieceType>(piece & (~WHITE));
}

inline Color pieceColor(Piece piece)
{
	// Upper bit
	return static_cast<Color>(piece & WHITE);
}

inline Color opponentColor(Color color)
{
	return static_cast<Color>(color ^ WHITE);
}

inline int pawnDirection(Color color)
{
	return color == WHITE ? 1 : -1;
}
#endif
