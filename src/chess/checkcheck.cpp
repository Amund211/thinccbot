#include "checkcheck.h"

#include "board.h"
#include "states.h"
#include "attacks.h"
#include "../game.h"

#include <cmath>
#include <map>
#include <memory>
#include <array>
#include <stdexcept>


bool hasPawnMove(
	Gamestate const* statep,
	Color c,
	Coordinate pos,
	std::unique_ptr<Coordinate>& mustKill,
	std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions
)
{
	int direction = pawnDirection(c);
	Coordinate target;
	auto pinnedElement = pinnedPositions.find(pos);

	for (int side=-1; side<=1; side+=2) {
		target = pos + Coordinate{direction, side};
		if (!target.isValid())
			continue;

		if (pinnedElement != pinnedPositions.end() && !pinnedElement->second.contains(target))
			continue;

		Piece targetPiece = statep->board.get(target);

		if (pieceColor(targetPiece) == opponentColor(c) && targetPiece != NONE) {
			// Attack a piece
			if (mustKill && target != *mustKill)
				continue;
			if (mustBlock && !mustBlock->contains(target))
				continue;

			return true;
		} else if (target == statep->passantSquare) {
			if (mustKill && statep->passantSquare - Delta{direction, 0} != *mustKill)
				continue;
			if (mustBlock && !mustBlock->contains(target))
				continue;

			return true;
		}
	}

	target = pos + Coordinate{direction, 0};
	if (statep->board.get(target) == NONE) {
		if (mustKill)
			return false;

		if (pinnedElement != pinnedPositions.end() && !pinnedElement->second.contains(target))
			// If moving 1 step breaks the pin, so will moving 2 steps
			return false;

		// Move one step
		if (!mustBlock || mustBlock->contains(target)) {
			return true;
		}

		if (pos.rank == (c == WHITE ? 1 : 6)) {
			if (!mustBlock || mustBlock->contains(target)) {
				return true;
			}
		}
	}

	// Found no legal moves
	return false;
}

bool hasKnightMove(
	Gamestate const* statep,
	Color c,
	Coordinate pos,
	std::unique_ptr<Coordinate>& mustKill,
	std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions
)
{
	for (Coordinate offset : knightMoves) {
		Coordinate target = pos + offset;
		if (!target.isValid())
			continue;
		if (mustKill && target != *mustKill)
			continue;
		if (mustBlock && !mustBlock->contains(target))
			continue;

		auto it = pinnedPositions.find(pos);
		if (it != pinnedPositions.end() && !it->second.contains(target))
			continue;

		Piece targetPiece = statep->board.get(target);
		// Empty squares are black, but this does not matter in this case
		if (targetPiece == NONE || pieceColor(targetPiece) != c) {
			return true;
		}
	}

	// Found no legal moves
	return false;
}

bool hasBishopMove(
	Gamestate const* statep,
	Color c,
	Coordinate pos,
	std::unique_ptr<Coordinate>& mustKill,
	std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions
)
{
	auto pinnedElement = pinnedPositions.find(pos);
	bool isPinned = pinnedElement != pinnedPositions.end();

#if 0
	// Not sure I want these
	if (mustKill && !(*mustKill - pos).isDiagonal())
		return false;
	if (isPinned && !pinnedElement->second.step.isDiagonal())
		return false;

#endif
	for (int rankDirection=-1; rankDirection<=1; rankDirection+=2) {
		for (int fileDirection=-1; fileDirection<=1; fileDirection+=2) {
			Delta step {rankDirection, fileDirection};
			for (Coordinate target {pos.rank + step.rank, pos.file + step.file}; target.isValid(); target += step) {
				bool cantMove = (mustKill && target != *mustKill) ||
					(mustBlock && !mustBlock->contains(target)) ||
					(isPinned && !pinnedElement->second.contains(target));

				Piece targetPiece = statep->board.get(target);
				// Empty squares are black, but this does not matter in this case
				if (targetPiece == NONE) {
					if (cantMove)
						continue;
					// Move
					return true;
				} else if (pieceColor(targetPiece) != c) {
					if (cantMove)
						continue;
					// Attack
					return true;
				} else {
					// Blocked by friendly piece, can't move further in this direction
					return false;
				}
			}
		}
	}
	return false;
}

bool hasRookMove(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	std::unique_ptr<Coordinate>& mustKill,
	std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions
)
{
	auto pinnedElement = pinnedPositions.find(pos);
	bool isPinned = pinnedElement != pinnedPositions.end();

#if 0
	// Not sure I want these
	if (mustKill && !(*mustKill - pos).isStraight())
		return false;
	if (isPinned && !pinnedElement->second.step.isStraight())
		return false;

#endif
	// Whether to move in rank or file
	for (int rank=0; rank<=1; rank++) {
		for (int direction=-1; direction<=1; direction+=2) {
			Delta step {rank*direction, (!rank)*direction};
			for (Coordinate target {pos.rank + step.rank, pos.file + step.file}; target.isValid(); target += step) {
				bool cantMove = (mustKill && target != *mustKill) ||
					(mustBlock && !mustBlock->contains(target)) ||
					(isPinned && !pinnedElement->second.contains(target));

				Piece targetPiece = statep->board.get(target);
				// Empty squares are black, but this does not matter in this case
				if (targetPiece == NONE) {
					if (cantMove)
						continue;
					// Move
					return true;
				} else if (pieceColor(targetPiece) != c) {
					if (cantMove)
						break;
					// Attack
					return true;
				} else {
					// Blocked by friendly piece, can't move further in this direction
					return false;
				}
			}
		}
	}
	return false;
}

bool hasQueenMove(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	std::unique_ptr<Coordinate>& mustKill,
	std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions
)
{
	return hasBishopMove(statep, c, pos, mustKill, mustBlock, pinnedPositions) ||
		hasRookMove(statep, c, pos, mustKill, mustBlock, pinnedPositions);
}

bool hasKingMove(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	const std::array<bool, 10>& attackedSquares
)
{
	for (auto it = kingMoveOrder.begin(); it != kingMoveOrder.end(); it++) {
		Coordinate target = pos + it->first;
		if (!target.isValid())
			continue;

		if (attackedSquares[it->second])
			// Square is attacked by the opponent
			continue;

		if (std::abs(it->first.file) == 2)
			// If you can castle you can also just move one square
			// in that direction
			continue;

		Piece targetPiece = statep->board.get(target);
		// Empty squares are black, but this does not matter in this case
		if (targetPiece == NONE || pieceColor(targetPiece) != c) {
			return true;
		}
	}

	// Found no legal moves
	return false;
}

// checkAttack and checkGuarded should go somewhere (board.cpp)
// and use less king-centric lingo so that it can be used in other circumstances
// Actually checkGuarded should be checkKingGuarded

GameStatus getGameStatus(Gamestate const* statep)
{
	if (statep->rule50Ply > 150)
		// Forced game end after 75 moves w/o captures/pawn moves
		return GameStatus::DRAW;

	Color toMove = statep->whiteToMove ? WHITE : BLACK;
	Coordinate kingPos = findKing(statep->board, toMove);

	std::array<bool, 10> attackedSquares = {
		false, false, false, false, false, false, false, false, false, false
	};
	std::unique_ptr<Coordinate> mustKill;
	std::unique_ptr<Line> mustBlock;
	std::map<Coordinate, Line> pinnedPositions;

	// Amount of checks on the king
	unsigned int amtChecks = getAttacks(
		// Parameters
		statep->board,
		kingPos,
		toMove,
		// Output
		attackedSquares,
		mustKill,
		mustBlock,
		pinnedPositions
	);

	// 2+ attackers -> only king-moves can get out of check
	if (amtChecks >= 2) {
		if (hasKingMove(statep, toMove, kingPos, attackedSquares))
			return GameStatus::UNDECIDED;
		else
			return GameStatus::WIN;
	}

	bool hasMoves = false;

	for (int rank=0; rank<8; rank++) {
		if (hasMoves)
			break;
		for (int file=0; file<8; file++) {
			if (hasMoves)
				break;
			Piece p = statep->board.get({rank, file});
			if (p != NONE && pieceColor(p) == toMove) {
				switch (pieceType(p)) {
					case PAWN:
						hasMoves = hasPawnMove(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions);
						break;
					case KNIGHT:
						hasMoves = hasKnightMove(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions);
						break;
					case BISHOP:
						hasMoves = hasBishopMove(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions);
						break;
					case ROOK:
						hasMoves = hasRookMove(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions);
						break;
					case QUEEN:
						hasMoves = hasQueenMove(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions);
						break;
					case KING:
						hasMoves = hasKingMove(statep, toMove, kingPos, attackedSquares);
						break;
					default:
						throw std::invalid_argument("Invalid piece on board");
						break;
				}
			}
		}
	}


	if (hasMoves) {
		if (statep->rule50Ply >= 150)
			// Forced game end after 75 moves w/o captures/pawn moves
			return GameStatus::DRAW;
		return GameStatus::UNDECIDED;
	}

	if (amtChecks)
		return GameStatus::WIN;
	else
		return GameStatus::DRAW;
}
