#include <cmath>
#include <map>
#include <memory>
#include <array>
#include <stdexcept>

#include "board.h"
#include "states.h"
#include "attacks.h"
#include "../game.h"

Gamestate* createState(Gamestate const* current, std::vector<Gamestate*>& gamestates)
{
	// Insert a copy of the current state into gamestates
	Gamestate* newstatep = new Gamestate{*current};
	gamestates.push_back(newstatep);

	// Update toMove
	newstatep->whiteToMove = !newstatep->whiteToMove;

	// Update en passant
	// This is probably faster than creating a new instance??
	newstatep->passantSquare.rank = -1;
	newstatep->passantSquare.file = -1;

	// Update ply since capture/pawn move
	newstatep->rule50Ply++;

	return newstatep;
}

void genPawnMoves(
	Gamestate const* statep,
	Color c,
	Coordinate pos,
	std::unique_ptr<Coordinate>& mustKill,
	std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
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

			if (target.rank == (c == WHITE ? 7 : 0)) {
				// Also promote
				for (Piece promotion=QUEEN; promotion>=KNIGHT; promotion--) {
					actions.push_back(new Action{pos, target, promotion});

					Gamestate* newstatep = createState(statep, gamestates);
					newstatep->board.set(pos, NONE);
					newstatep->board.set(target, c | promotion);
					// Pawn move & capture
					newstatep->rule50Ply = 0;
				}
			} else {
				actions.push_back(new Action{pos, target});

				Gamestate* newstatep = createState(statep, gamestates);
				newstatep->board.move(pos, target);
				// Pawn move & capture
				newstatep->rule50Ply = 0;
			}
		} else if (target == statep->passantSquare) {
			if (mustKill && statep->passantSquare - Delta{direction, 0} != *mustKill)
				continue;
			if (mustBlock && !mustBlock->contains(target))
				continue;
			// En passant
			actions.push_back(new Action{pos, target});

			Gamestate* newstatep = createState(statep, gamestates);
			newstatep->board.move(pos, target);
			newstatep->board.set(pos - Coordinate{direction, 0}, NONE);
			// Pawn move & capture
			newstatep->rule50Ply = 0;
		}
	}

	target = pos + Coordinate{direction, 0};
	if (statep->board.get(target) == NONE) {
		if (mustKill)
			return;

		if (pinnedElement != pinnedPositions.end() && !pinnedElement->second.contains(target))
			// If moving 1 step breaks the pin, so will moving 2 steps
			return;

		// Move one step
		if (!mustBlock || mustBlock->contains(target)) {
			if (target.rank == (c == WHITE ? 7 : 0)) {
				// Also promote
				for (Piece promotion=QUEEN; promotion>=KNIGHT; promotion--) {
					actions.push_back(new Action{pos, target, promotion});

					Gamestate* newstatep = createState(statep, gamestates);
					newstatep->board.set(pos, NONE);
					newstatep->board.set(target, c | promotion);
					// Pawn move
					newstatep->rule50Ply = 0;
				}
			} else {
				actions.push_back(new Action{pos, target});

				Gamestate* newstatep = createState(statep, gamestates);
				newstatep->board.move(pos, target);
				// Pawn move
				newstatep->rule50Ply = 0;
			}
		}

		if (pos.rank == (c == WHITE ? 1 : 6)) {
			if (!mustBlock || mustBlock->contains(target)) {
				target = pos + Coordinate{2*direction, 0};
				if (statep->board.get(target) == NONE) {
					// Move two steps
					actions.push_back(new Action{pos, target});

					Gamestate* newstatep = createState(statep, gamestates);
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
	Coordinate pos,
	std::unique_ptr<Coordinate>& mustKill,
	std::unique_ptr<Line>& mustBlock,
	const std::map<Coordinate, Line>& pinnedPositions,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
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
			// Move or attack
			actions.push_back(new Action{pos, target});

			Gamestate* newstatep = createState(statep, gamestates);
			newstatep->board.move(pos, target);
			if (targetPiece != NONE)
				// Capture
				newstatep->rule50Ply = 0;
		}
	}

}

// genRookMoves
// if file == 7, rank == startingrank -> kingside = false

void genKingMoves(
	Gamestate const* statep,
	Color c,
	const Coordinate& pos,
	const std::array<bool, 8>& attackedSquares,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
)
{
	for (auto it = kingMoveOrder.begin(); it != kingMoveOrder.end(); it++) {
		Coordinate target = pos + it->first;
		if (!target.isValid())
			continue;

		//std::cerr << target.toString() << ": " << attackedSquares[it->second] << std::endl;

		if (attackedSquares[it->second])
			// Square is attacked by the opponent
			continue;

		Piece targetPiece = statep->board.get(target);
		// Empty squares are black, but this does not matter in this case
		if (targetPiece == NONE || pieceColor(targetPiece) != c) {
			// Move or attack
			actions.push_back(new Action{pos, target});

			Gamestate* newstatep = createState(statep, gamestates);
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

unsigned int getActions(
	Gamestate const* statep,
	std::vector<Gamestate*>& gamestates,
	std::vector<Action*>& actions
)
{
	// Both in stalemate and in mate no moves should be generated
	if (statep->rule50Ply >= 150)
		// Forced game end after 75 moves w/o captures/pawn moves
		return 0;

	Color toMove = statep->whiteToMove ? WHITE : BLACK;
	Coordinate kingPos = findKing(statep->board, toMove);

	std::array<bool, 8> attackedSquares = {
		false, false, false, false, false, false, false, false
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
		genKingMoves(statep, toMove, kingPos, attackedSquares, gamestates, actions);
		return actions.size();
	}

	for (int rank=0; rank<8; rank++) {
		for (int file=0; file<8; file++) {
			Piece p = statep->board.get({rank, file});
			if (p != NONE && pieceColor(p) == toMove) {
				// Must keep blocking (same as above?)
				// 	If you are on a line, you must stay on that line
				// 	If not - see above
				// 	Any piece

				// One of
				// 	Must kill - target = target, but keep en passant in mind
				// 	Either pawns or knight

				// 	Must block
				// 	Bishops, rooks, queens


				switch (pieceType(p)) {
					case PAWN:
						genPawnMoves(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions, gamestates, actions);
						break;
					case KNIGHT:
						genKnightMoves(statep, toMove, {rank, file}, mustKill, mustBlock, pinnedPositions, gamestates, actions);
						break;
					case BISHOP:
						break;
					case ROOK:
						break;
					case QUEEN:
						break;
					case KING:
						genKingMoves(statep, toMove, kingPos, attackedSquares, gamestates, actions);
						break;
					default:
						throw std::invalid_argument("Invalid piece on board");
						break;
				}
			}
		}
	}

	return actions.size();
}
