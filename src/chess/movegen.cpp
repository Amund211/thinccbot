//#define NDEBUG

#include <cmath>
#include <map>
#include <memory>
#include <array>
#include <stdexcept>
#include <cassert>

#include "board.h"
#include "states.h"
#include "attacks.h"
#include "../game.h"

Gamestate* createState(Gamestate const* current, std::vector<Gamestate*>& gamestates, const Coordinate& target)
{
	// Insert a copy of the current state into gamestates
	Gamestate* newstatep = new Gamestate{*current};
	gamestates.push_back(newstatep);

	// Test if a rook is being captured
	if (target.rank == (newstatep->whiteToMove ? 7 : 0)) {
		// Unset queen-/kingside castling-rights if capturing on a or h
		switch (target.file) {
			case 0:
				(newstatep->whiteToMove ? newstatep->blackCastle : newstatep->whiteCastle).queenside = false;
				break;
			case 7:
				(newstatep->whiteToMove ? newstatep->blackCastle : newstatep->whiteCastle).kingside = false;
				break;
			default:
				break;
		}
	}

	// Update toMove
	newstatep->whiteToMove = !newstatep->whiteToMove;

	// Update en passant
	newstatep->passantSquare.rank = -1;
	newstatep->passantSquare.file = -1;

	// Update ply since capture/pawn move
	newstatep->rule50Ply++;

	return newstatep;
}

void genPawnMoves(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	const std::unique_ptr<Coordinate>& mustKill,
	const std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
)
{
	int direction = pawnDirection(c);
	Coordinate target;
	auto pinnedElement = pinnedPositions.find(pos);
	bool isPinned = pinnedElement != pinnedPositions.end();

	for (int side=-1; side<=1; side+=2) {
		target = pos + Coordinate{direction, side};
		if (!target.isValid())
			continue;

		if (isPinned && !pinnedElement->second.contains(target))
			continue;

		Piece targetPiece = statep->board.get(target);

		if (pieceColor(targetPiece) == opponentColor(c) && targetPiece != NONE) {
			// Attack a piece
			assert(pieceType(targetPiece) != KING);
			if (mustKill && target != *mustKill)
				continue;
			if (mustBlock && !mustBlock->contains(target))
				continue;

			if (target.rank == (c == WHITE ? 7 : 0)) {
				// Also promote
				for (Piece promotion=QUEEN; promotion>=KNIGHT; promotion--) {
					actions.push_back(new Action{pos, target, promotion});

					Gamestate* newstatep = createState(statep, gamestates, target);
					newstatep->board.set(pos, NONE);
					newstatep->board.set(target, c | promotion);
					// Pawn move & capture
					newstatep->rule50Ply = 0;
				}
			} else {
				actions.push_back(new Action{pos, target});

				Gamestate* newstatep = createState(statep, gamestates, target);
				newstatep->board.move(pos, target);
				// Pawn move & capture
				newstatep->rule50Ply = 0;
			}
		} else if (target == statep->passantSquare) {
			Coordinate pawnPos {statep->passantSquare - Delta{direction, 0}};
			if (mustKill && pawnPos != *mustKill)
				continue;
			if (mustBlock && !mustBlock->contains(target))
				continue;
			// En passant
			actions.push_back(new Action{pos, target});

			Gamestate* newstatep = createState(statep, gamestates, target);
			newstatep->board.move(pos, target);
			newstatep->board.set(pawnPos, NONE);
			// Pawn move & capture
			newstatep->rule50Ply = 0;
		}
	}

	target = pos + Coordinate{direction, 0};
	if (statep->board.get(target) == NONE) {
		if (mustKill)
			return;

		if (isPinned && !pinnedElement->second.contains(target))
			// If moving 1 step breaks the pin, so will moving 2 steps
			return;

		// Move one step
		if (!mustBlock || mustBlock->contains(target)) {
			if (target.rank == (c == WHITE ? 7 : 0)) {
				// Also promote
				for (Piece promotion=QUEEN; promotion>=KNIGHT; promotion--) {
					actions.push_back(new Action{pos, target, promotion});

					Gamestate* newstatep = createState(statep, gamestates, target);
					newstatep->board.set(pos, NONE);
					newstatep->board.set(target, c | promotion);
					// Pawn move
					newstatep->rule50Ply = 0;
				}
			} else {
				actions.push_back(new Action{pos, target});

				Gamestate* newstatep = createState(statep, gamestates, target);
				newstatep->board.move(pos, target);
				// Pawn move
				newstatep->rule50Ply = 0;
			}
		}

		if (pos.rank == (c == WHITE ? 1 : 6)) {
			target = pos + Coordinate{2*direction, 0};
			if (!mustBlock || mustBlock->contains(target)) {
				if (statep->board.get(target) == NONE) {
					// Move two steps
					actions.push_back(new Action{pos, target});

					Gamestate* newstatep = createState(statep, gamestates, target);
					newstatep->board.move(pos, target);
					newstatep->passantSquare = target - Coordinate{direction, 0};
					// Pawn move
					newstatep->rule50Ply = 0;
				}
			}
		}
	}
}

void genKnightMoves(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	const std::unique_ptr<Coordinate>& mustKill,
	const std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
)
{
	auto pinnedElement = pinnedPositions.find(pos);
	bool isPinned = pinnedElement != pinnedPositions.end();
	for (Coordinate offset : knightMoves) {
		Coordinate target = pos + offset;
		if (!target.isValid())
			continue;
		if (mustKill && target != *mustKill)
			continue;
		if (mustBlock && !mustBlock->contains(target))
			continue;

		if (isPinned && !pinnedElement->second.contains(target))
			continue;

		Piece targetPiece = statep->board.get(target);
		// Empty squares are black, but this does not matter in this case
		if (targetPiece == NONE || pieceColor(targetPiece) != c) {
			assert(pieceType(targetPiece) != KING);
			// Move or attack
			actions.push_back(new Action{pos, target});

			Gamestate* newstatep = createState(statep, gamestates, target);
			newstatep->board.move(pos, target);
			if (targetPiece != NONE)
				// Capture
				newstatep->rule50Ply = 0;
		}
	}

}

void genBishopMoves(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	const std::unique_ptr<Coordinate>& mustKill,
	const std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
)
{
	auto pinnedElement = pinnedPositions.find(pos);
	bool isPinned = pinnedElement != pinnedPositions.end();

	if (mustKill && !(*mustKill - pos).isDiagonal())
		return;
	if (isPinned && !pinnedElement->second.step.isDiagonal())
		return;

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
					actions.push_back(new Action{pos, target});

					Gamestate* newstatep = createState(statep, gamestates, target);
					newstatep->board.move(pos, target);
				} else if (pieceColor(targetPiece) != c) {
					assert(pieceType(targetPiece) != KING);
					if (!cantMove) {
						// Attack
						actions.push_back(new Action{pos, target});

						Gamestate* newstatep = createState(statep, gamestates, target);
						newstatep->board.move(pos, target);
						// Capture
						newstatep->rule50Ply = 0;
					}

					// Blocked by enemy piece, can't move further in this direction
					break;
				} else {
					// Blocked by friendly piece, can't move further in this direction
					break;
				}
			}
		}
	}
}

void setCastle(Gamestate* statep, Color c, const Coordinate& pos)
{
	// Assumes the rook is on its starting rank
	assert(pos.rank == (c == WHITE ? 0 : 7));
	// Unset queen-/kingside castling-rights if departing from a or h
	switch (pos.file) {
		case 0:
			(c == WHITE ? statep->whiteCastle : statep->blackCastle).queenside = false;
			break;
		case 7:
			(c == WHITE ? statep->whiteCastle : statep->blackCastle).kingside = false;
			break;
		default:
			break;
	}
}

void genRookMoves(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	const std::unique_ptr<Coordinate>& mustKill,
	const std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
)
{
	auto pinnedElement = pinnedPositions.find(pos);
	bool isPinned = pinnedElement != pinnedPositions.end();

	int homeRank = c == WHITE ? 0 : 7;

	if (mustKill && !(*mustKill - pos).isStraight())
		return;
	if (isPinned && !pinnedElement->second.step.isStraight())
		return;

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
					actions.push_back(new Action{pos, target});

					Gamestate* newstatep = createState(statep, gamestates, target);
					newstatep->board.move(pos, target);
					if (pos.rank == homeRank)
						// Unset castling avaliability
						setCastle(newstatep, c, pos);

				} else if (pieceColor(targetPiece) != c) {
					assert(pieceType(targetPiece) != KING);
					if (!cantMove) {
						// Attack
						actions.push_back(new Action{pos, target});

						Gamestate* newstatep = createState(statep, gamestates, target);
						newstatep->board.move(pos, target);
						// Capture
						newstatep->rule50Ply = 0;

						if (pos.rank == homeRank)
							// Unset castling avaliability
							setCastle(newstatep, c, pos);
					}

					// Blocked by enemy piece, can't move further in this direction
					break;
				} else {
					// Blocked by friendly piece, can't move further in this direction
					break;
				}
			}
		}
	}
}

void genQueenMoves(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	const std::unique_ptr<Coordinate>& mustKill,
	const std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
)
{
	genBishopMoves(statep, c, pos, mustKill, mustBlock, pinnedPositions, gamestates, actions);
	genRookMoves(statep, c, pos, mustKill, mustBlock, pinnedPositions, gamestates, actions);
}

void genKingMoves(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	const std::array<bool, 10>& attackedSquares,
	bool inCheck,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
)
{
	const Castle& myCastle = (c == WHITE ? statep->whiteCastle : statep->blackCastle);
	bool mayCastle = !inCheck &&
		pos.file == 4 &&
		(pos.rank == (c == WHITE ? 0 : 7));

	for (auto it = kingMoveOrder.begin(); it != kingMoveOrder.end(); it++) {
		Coordinate target = pos + it->first;
		if (!target.isValid())
			continue;

		if (attackedSquares[it->second])
			// Square is attacked by the opponent
			continue;

		Piece targetPiece = statep->board.get(target);

		if (std::abs(it->first.file) == 2) {
			// Castle
			if (it->first.file < 0) {
				// Queenside
				if (
					mayCastle &&
					myCastle.queenside &&
					statep->board.get(target) == NONE &&
					statep->board.get(target + Coordinate{0, 1}) == NONE &&
					statep->board.get(target - Coordinate{0, 1}) == NONE &&
					!attackedSquares[kingMoveOrder.find({0, -1})->second]
				) {
					actions.push_back(new Action{pos, target});

					Gamestate* newstatep = createState(statep, gamestates, target);
					// Move the king
					newstatep->board.move(pos, target);
					// Move the rook
					newstatep->board.move({(c == WHITE ? 0 : 7), 0}, target + Coordinate{0, 1});

					if (c == WHITE)
						newstatep->whiteCastle = {false, false};
					else
						newstatep->blackCastle = {false, false};
				}

			} else {
				// Kingside
				if (
					mayCastle &&
					myCastle.kingside &&
					statep->board.get(target) == NONE &&
					statep->board.get(target + Coordinate{0, -1}) == NONE &&
					!attackedSquares[kingMoveOrder.find({0, 1})->second]
				) {
					actions.push_back(new Action{pos, target});

					Gamestate* newstatep = createState(statep, gamestates, target);
					// Move the king
					newstatep->board.move(pos, target);
					// Move the rook
					newstatep->board.move({(c == WHITE ? 0 : 7), 7}, target - Coordinate{0, 1});

					if (c == WHITE)
						newstatep->whiteCastle = {false, false};
					else
						newstatep->blackCastle = {false, false};
				}
			}
		// Empty squares are black, but this does not matter in this case
		} else if (targetPiece == NONE || pieceColor(targetPiece) != c) {
			// Move or attack
			actions.push_back(new Action{pos, target});

			Gamestate* newstatep = createState(statep, gamestates, target);
			newstatep->board.move(pos, target);

			// Can't castle after moving king
			if (c == WHITE)
				newstatep->whiteCastle = {false, false};
			else
				newstatep->blackCastle = {false, false};

			if (targetPiece != NONE)
				// Capture
				newstatep->rule50Ply = 0;
		}
	}

}

void genChildren(
	Gamestate const* statep,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
)
{
	// Both in stalemate and in mate no moves should be generated
	if (statep->rule50Ply >= 150)
		// Forced game end after 75 moves w/o captures/pawn moves
		return;

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
		genKingMoves(statep, toMove, kingPos, attackedSquares, true, gamestates, actions);
		return;
	}

	for (int rank=0; rank<8; rank++) {
		for (int file=0; file<8; file++) {
			Piece p = statep->board.get({rank, file});
			if (p != NONE && pieceColor(p) == toMove) {
				switch (pieceType(p)) {
					case PAWN:
						genPawnMoves(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions, gamestates, actions);
						break;
					case KNIGHT:
						genKnightMoves(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions, gamestates, actions);
						break;
					case BISHOP:
						genBishopMoves(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions, gamestates, actions);
						break;
					case ROOK:
						genRookMoves(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions, gamestates, actions);
						break;
					case QUEEN:
						genQueenMoves(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions, gamestates, actions);
						break;
					case KING:
						genKingMoves(statep, toMove, kingPos, attackedSquares, amtChecks != 0, gamestates, actions);
						break;
					default:
						throw std::invalid_argument("Invalid piece on board");
						break;
				}
			}
		}
	}

	return;
}
