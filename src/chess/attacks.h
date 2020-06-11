#ifndef ATTACKS_H_INCLUDED
#define ATTACKS_H_INCLUDED

#include "pieces.h"
#include "board.h"

#include <array>
#include <map>
#include <memory>

enum class AttackStatus
{
	CLEAR,
	ATTACKED,
	PINNED
};

const std::array<Delta, 8> knightMoves = {
	Delta{ 2,  1},
	Delta{ 1,  2},
	Delta{-1,  2},
	Delta{-2,  1},
	Delta{-2, -1},
	Delta{-1, -2},
	Delta{ 1, -2},
	Delta{ 2, -1}
};

const std::map<Delta, unsigned int> kingMoveOrder {
	{{ 1,  0}, 0},
	{{ 1,  1}, 1},
	{{ 0,  1}, 2},
	{{-1,  1}, 3},
	{{-1,  0}, 4},
	{{-1, -1}, 5},
	{{ 0, -1}, 6},
	{{ 1, -1}, 7}
};

AttackStatus checkAttack(
	const Board& b,
	const Coordinate& kingPos,
	const Coordinate& attackerPos,
	Color opponent,
	Coordinate& pinnedPos
);

void checkGuarded(
	const Board& b,
	const Coordinate& target,
	const Coordinate& kingPos,
	const Coordinate& attackerPos,
	std::array<bool, 8>& attackedSquares
);

unsigned int getAttacks(
	// Parameters
	const Board& board,
	const Coordinate& kingPos,
	Color toMove,
	// Output
	// Bool array for the squares around the king
	// Ordered according to kingMoveOrder
	std::array<bool, 8>& attackedSquares,

	// If there is an associated object, all valid moves must kill the piece at that pos
	std::unique_ptr<Coordinate>& mustKill,

	// If there is an associated object, all valid moves must move to that line
	std::unique_ptr<Line>& mustBlock,

	// Map from pinned postions to the line they are pinned on
	std::map<Coordinate, Line>& pinnedPositions
);

#endif
