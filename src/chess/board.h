#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "pieces.h"

#include <cmath>
#include <string>
#include <array>
#include <iostream>

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
	Coordinate(std::string SAN);
	Coordinate() = default;
	//Coordinate(const Coordinate&) = default;

	inline bool isValid() const {return rank<8 && rank>=0 && file<8 && file>=0;}

	// Useful for deltas
	inline int infNorm() const {return std::max(std::abs(rank), std::abs(file));}
	inline bool isStraight() const {return (rank == 0) != (file == 0);}
	inline bool isDiagonal() const {return std::abs(rank) == std::abs(file);}
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
		// delta must be straight or diagonal
		Delta delta {end-start};
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

		/* Missing check for paralellism
		// Direction- and step- vector are colinear
		return (
			static_cast<float>(direction.rank)/step.rank ==
			static_cast<float>(direction.file)/step.file
		);
		*/
		// Scaled direction-vector matches step-vector
		return (
			static_cast<float>(direction.rank)/Linf == step.rank &&
			static_cast<float>(direction.file)/Linf == step.file
		);

		/*
		for (unsigned int i=0; i<=maxSteps; i++)
			if (pos.rank == start.rank + step.rank * i &&
				pos.file == start.file + step.file * i)
				return true;
		return false;
		*/
	}
};

struct Board
{
	uint8_t _board[32]; // 64 nibbles

	Piece get(Coordinate pos) const;
	void set(Coordinate pos, Piece piece);
	void move(Coordinate from, Coordinate to);

	void print(Color perspective=WHITE, bool colorTerminal=false, std::ostream& stream=std::cout);
};

Coordinate findKing(const Board& b, Color c);

#endif
