#ifndef STATES_H_INCLUDED
#define STATES_H_INCLUDED

#include "board.h"
#include "pieces.h"

#include <string>
#include <cstdint>
#include <cmath>

#include <iostream>

extern const std::string STARTING_FEN;

struct Action
{
	Coordinate from;
	Coordinate to;
	Piece promotionPiece;

	Action(Coordinate from, Coordinate to, Piece promotionPiece=NONE);

	std::string toAN() const;
	std::string toString() const;

	bool operator ==(const Action& b) const;
};

struct Castle
{
	bool kingside:1;
	bool queenside:1;

	inline bool canCastle() const { return kingside || queenside; }
};

struct Gamestate
{
	// Consider changing to int:1, and do some shift magic in trees
	bool whiteToMove;
	Board board;
	Castle whiteCastle;
	Castle blackCastle;
	Coordinate passantSquare;
	uint8_t rule50Ply; // Can not exceed 150

	Gamestate(std::string FEN);
	Gamestate();

	std::string toFEN() const;
	inline std::string toString() const { return toFEN(); }
};

#endif
