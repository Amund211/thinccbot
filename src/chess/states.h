#ifndef STATES_H_INCLUDED
#define STATES_H_INCLUDED

#include "pieces.h"

#include <string>
#include <cstdint>
#include <cmath>

struct Coordinate;
// Alias for Coordinate
typedef Coordinate Delta;

struct Coordinate
{
	// Big idea: reduce these to int8_t
	int rank;
	int file;

	//inline unsigned int toOffset() const { return rank*8 + file; };
	Coordinate(int rank, int file): rank{rank}, file{file} {}
	Coordinate() = default;
	//Coordinate(const Coordinate&) = default;

	inline bool isValid() const {return rank<8 && rank>=0 && file<8 && file>=0;}

	// Useful for deltas
	inline int infNorm() const {return std::max(std::abs(rank), std::abs(file));}
	inline bool isStraight() const {return (rank == 0) != (file == 0);}
	inline bool isDiagonal() const {return std::abs(rank) == std::abs(file);}
	Delta step() const;

	std::string toString() const;

	Coordinate operator +(const Coordinate& b) const;
	Coordinate& operator +=(const Coordinate& b);
	Coordinate operator -(const Coordinate& b) const;
	Coordinate& operator -=(const Coordinate& b);
	Coordinate operator -() const;

	bool operator ==(const Coordinate& b) const;
	bool operator !=(const Coordinate& b) const;

	// For less<Coordinate>
	bool operator <(const Coordinate& b) const;
};


struct Castle
{
	bool kingside:1;
	bool queenside:1;

	inline bool canCastle() const { return kingside || queenside; }
};

struct Board
{
	uint8_t _board[32]; // 64 nibbles

	Piece get(Coordinate pos) const;
	void set(Coordinate pos, Piece piece);
	void move(Coordinate from, Coordinate to);
};

struct Action
{
	Coordinate from;
	Coordinate to;
	Piece promotionPiece;

	Action(Coordinate from, Coordinate to, Piece promotionPiece=NONE);

	bool operator ==(const Action& b) const;
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
};

float materialCount(const Board& b);

Coordinate findKing(const Board& b, Color c);

std::string aToString(Action const* actionp);
std::string sToString(Gamestate const* statep);

#endif
