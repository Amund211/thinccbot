#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "pieces.h"

#include <cmath>
#include <string>
#include <array>
#include <iostream>
#include <cassert>

struct Coordinate;
// Alias for Coordinate
typedef Coordinate Delta;

struct Coordinate
{
	int rank;
	int file;

	Coordinate(int rank, int file): rank{rank}, file{file} {}
	Coordinate(std::string SAN);
	Coordinate() = default;

	inline bool isValid() const {return rank<8 && rank>=0 && file<8 && file>=0;}

	// Useful for deltas
	inline int infNorm() const {return std::max(std::abs(rank), std::abs(file));}
	inline bool isStraight() const {return (rank == 0) != (file == 0);}
	inline bool isDiagonal() const {return rank != 0 && std::abs(rank) == std::abs(file);}
	Delta step() const;

	std::string toSAN() const;
	std::string toString() const { return toSAN(); }

	Coordinate operator +(const Coordinate& b) const;
	Coordinate& operator +=(const Coordinate& b);
	Coordinate operator -(const Coordinate& b) const;
	Coordinate& operator -=(const Coordinate& b);
	Coordinate operator -() const;

	bool operator ==(const Coordinate& b) const;
	bool operator !=(const Coordinate& b) const;
};

namespace std
{
	template<> struct less<Delta>
	{
		bool operator() (const Delta& lhs, const Delta& rhs) const
		{
			// Orders Deltas with Linf < 16
			return (32*lhs.rank + lhs.file) < (32*rhs.rank + rhs.file);
		}
	};
}

struct Line
{
	// Represents a line-segment on the board
	Coordinate start;
	Delta step;
	int maxSteps;

	Line(Coordinate start, Coordinate end)
		: start{start}
	{
		Delta delta {end-start};
		// delta must be straight or diagonal
		assert(delta.isDiagonal() || delta.isStraight());

		maxSteps = delta.infNorm();
		step = {delta.rank / maxSteps, delta.file/maxSteps};
	}

	bool contains(Coordinate pos) const
	{
		// Return true if pos is on the line, including the endpoint, but not including the start
		Delta direction = pos - start;
		int Linf = direction.infNorm();
		// The inf-norm of the direction-vector corresponds to the required amount
		// of steps of a step-vector with inf-norm 1
		if (Linf > maxSteps || Linf == 0)
			return false;

		// Scaled direction-vector matches step-vector
		return (
			static_cast<float>(direction.rank)/Linf == step.rank &&
			static_cast<float>(direction.file)/Linf == step.file
		);
	}
};

struct Board
{
	uint8_t _board[64];

	Piece get(Coordinate pos) const;
	void set(Coordinate pos, Piece piece);
	void move(Coordinate from, Coordinate to);

	void print(Color perspective=WHITE, bool colorTerminal=false, std::ostream& stream=std::cout) const;
};

Coordinate findKing(const Board& b, Color c);

#endif
