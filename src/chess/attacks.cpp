#include "attacks.h"

#include "pieces.h"
#include "board.h"

#include <array>
#include <cassert>

AttackStatus checkAttack(
	const Board& b,
	const Coordinate& kingPos,
	const Coordinate& attackerPos,
	Color opponent,
	Coordinate& pinnedPos
)
{
	// Return the AttackStatus of the king at pos kingPos due to the enemy piece
	// at pos attackerPos
	// If the return-value is AttackStatus::PINNED, pinnedPos is set to the
	// position of the pinned piece

	assert((attackerPos-kingPos).isDiagonal() || (attackerPos-kingPos).isStraight());
	// Assumes that kingPos and attackerPos are on the same diagonal/rank/file
	Delta step {(attackerPos-kingPos).step()};

	bool blocked = false;

	// Start look at all the spaces between the friendly king and the attacking piece
	for (Coordinate cur{kingPos + step}; cur != attackerPos; cur += step) {
		Piece tmpPiece = b.get(cur);
		if (pieceType(tmpPiece) != NONE) {
			if (pieceColor(tmpPiece) == opponent) {
				// An opposing piece is blocking
				// We are free to move any piece off the line
				return AttackStatus::CLEAR;
			} else {
				if (blocked) {
					// Two friendly pieces are blocking
					// We are free to move any piece off the line
					return AttackStatus::CLEAR;
				} else {
					pinnedPos = cur;
					blocked = true;
				}
			}
		}
	}

	if (blocked)
		// Exactly one friendly blocker and no hostile blockers
		return AttackStatus::PINNED;
	else
		return AttackStatus::ATTACKED;
}

void checkGuarded(
	const Board& b,
	const Coordinate& target,
	const Coordinate& kingPos,
	const Coordinate& attackerPos,
	std::array<bool, 8>& attackedSquares
)
{
	/*
	std::cerr << "Call to checkGuarded:\n"
		<< "kingPos: " << kingPos.toString() << "\n"
		<< "target: " << target.toString() << "\n"
		<< "attackerPos: " << attackerPos.toString() << std::endl;
	*/
	assert((attackerPos-target).isDiagonal() || (attackerPos-target).isStraight());

	// Assumes that target and attackerPos are on the same diagonal/rank/file
	Delta step {(target-attackerPos).step()};

	// Start look at all the spaces between the attacking piece and the target
	for (Coordinate cur{attackerPos + step}; cur != target; cur += step) {
		Delta kingMove {cur-kingPos};
		if (kingMove.infNorm() == 1)
			// Valid king-move
			attackedSquares[kingMoveOrder.at(kingMove)] = true;

		Piece tmpPiece = b.get(cur);
		if (pieceType(tmpPiece) != NONE) {
			// Further squares are blocked by this piece
			return;
		}
	}
	// Didn't return -> no blockers so far -> target is guarded
	attackedSquares[kingMoveOrder.at(target-kingPos)] = true;
}
