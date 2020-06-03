#include <cmath>

#include "states.h"
#include "../game.h"

Gamestate* createState(Gamestate const* current, std::vector<Gamestate*>& gamestates)
{
	// Insert a copy of the current state into gamestates
	Gamestate* newstatep = new Gamestate{*current};
	gamestates.push_back(newstatep);

	// Update toMove
	newstatep->whiteToMove = !newstatep->whiteToMove;

	// Update en passant
	newstatep->passantSquare = {0, 0};

	// Update ply since capture/pawn move
	newstatep->rule50Ply++;

	return newstatep;
}

void genPawnMoves(Gamestate const* statep, Color c, Coordinate pos, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions)
{
	int direction = pawnDirection(c);
	Coordinate target;

	for (int side=-1; side<=1; side+=2) {
		target = pos + Coordinate{direction, side};
		if (!target.isValid())
			continue;

		Piece targetPiece = statep->board.get(target);

		if (pieceColor(targetPiece) == opponentColor(c) && targetPiece != NONE) {
			// Attack a piece
			if (target.rank == (c == WHITE ? 7 : 0)) {
				// Also promote
				for (Piece promotion=KNIGHT; promotion<=QUEEN; promotion++) {
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
		// Move one step
		if (target.rank == (c == WHITE ? 7 : 0)) {
			// Also promote
			for (Piece promotion=KNIGHT; promotion<=QUEEN; promotion++) {
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

		if (pos.rank == (c == WHITE ? 1 : 6)) {
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

void genKnightMoves(Gamestate const* statep, Color c, Coordinate pos, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions)
{
	Coordinate target;
	for (int longDir=-1; longDir<=1; longDir+=2) {
		for (int shortDir=-1; shortDir<=1; shortDir+=2) {
			for (unsigned int longRank=0; longRank<=1; longRank++) {
				if (longRank)
					target = pos + Coordinate{2*longDir, shortDir};
				else
					target = pos + Coordinate{shortDir, 2*longDir};

				if (!target.isValid())
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
	}

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
		maxSteps = delta.infNorm();
		step = {delta.rank / maxSteps, delta.file/maxSteps};
	}

	bool contains(Coordinate pos)
	{
		Delta direction = pos - start;
		int Linf = direction.infNorm();
		// The inf-norm of the direction-vector corresponds to the required amount
		// of steps of a step-vector with inf-norm 1
		if (Linf > maxSteps)
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

bool pinned(const Board& b, const Coordinate& start, const Coordinate& end, Color opponent, Coordinate& pinnedPos)
{
	// HUGE TIP
	// PASS IN REFERENCE TO A SINGLE BOOL FIELD WHICH SAYS WHETHER THE SQUARE NEXT TO THE KING IS UNDER ATTACK

	// Return true if one of our pieces are pinned on this line
	// If this is the case pinnedPos is the position of that piece
	// Start is the position of the friendly king, and end is the position of the attacking piece

	// Assumes that start and end are on the same diagonal/rank/file
	Delta step {(end-start).step()};

	bool blocked = false;


	// Start look at all the spaces between the friendly king and the attacking piece
	for (Coordinate cur{start + step}; cur != end; cur += step) {
		Piece tmpPiece = b.get(cur);
		if (pieceType(tmpPiece) != NONE) {
			if (pieceColor(tmpPiece) == opponent) {
				// An opposing piece is blocking
				// We are free to move any piece off the line
				return false;
			} else {
				if (blocked) {
					// To friendly pieces are blocking
					// We are free to move any piece off the line
					return false;
				} else {
					pinnedPos = cur;
					blocked = true;
				}
			}
		}
	}

	// Exactly one friendly blocker and no hostile blockers -> keep that piece there
	return true;
}

bool gameOver(Gamestate const* statep);
unsigned int getActions(Gamestate const* statep, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions) {
	// Only need to check for mate - when in stalemate 0 moves are generated
	if (gameOver(statep)) {
		return 0;
	}

	Color toMove = statep->whiteToMove ? WHITE : BLACK;
#if 0
	Color opponent = opponentColor(toMove);
	int opponentDirection = pawnDirection(opponent);

	Coordinate mustKill;


	// Amount of checks on the king
	unsigned int amtChecks = 0;

	Coordinate kingPos = findKing(statep->board, toMove);

	for (int rank=0; rank<8; rank++) {
		if (amtChecks >= 2)
			break;
		for (int file=0; file<8; file++) {
			if (amtChecks >= 2)
				break;
			Piece p = statep->board.get({rank, file});
			if (p != NONE && pieceColor(p) == opponent) {
				Delta delta = kingPos - Coordinate{rank, file};
				switch (pieceType(p)) {
					case PAWN:
						if (
							delta == Coordinate{opponentDirection, 1} ||
							delta == Coordinate{opponentDirection, -1}
						) {
							amtChecks++;
							mustKill = {rank, file};
						}
						break;
					case KNIGHT:
						// Equivalient with valid knight-move when
						// rank and file are integers
						if (std::abs(delta.rank * delta.file) == 2) {
							amtChecks++;
							mustKill = {rank, file};
						}
						break;
					case BISHOP:
						if (delta.isDiagonal())
						break;
					case ROOK:
						if (delta.isDiagonal())
						break;
					case QUEEN:
						if (delta.isDiagonal() || delta.isStraight())
						break;
					case KING:
						break;
					default:
						continue;
				}
			}
		}
	}

	/*
	// 2+ attackers -> only king-moves can get out of check
	if (amtChecks >= 2) {
		genKingMoves(statep, toMove, {rank, file}, gamestates, actions);
		return actions.size();
	}
	*/
#endif

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
						genPawnMoves(statep, toMove, {rank, file}, gamestates, actions);
						break;
					case KNIGHT:
						genKnightMoves(statep, toMove, {rank, file}, gamestates, actions);
						break;
					case BISHOP:
						break;
					case ROOK:
						break;
					case QUEEN:
						break;
					case KING:
						break;
					default:
						continue;
				}
			}
		}
	}

	return actions.size();
}

float evaluation(Gamestate const* statep) {
	for (int rank=0; rank<8; rank++) {
		for (int file=0; file<8; file++) {
			Piece p = statep->board.get({rank, file});
			if (pieceType(p) == QUEEN) {
				if (pieceColor(p) == WHITE)
					return 100;
				else
					return -100;
			}
		}
	}
	return materialCount(statep->board);
}

bool gameOver(Gamestate const* statep)
{
	return std::abs(evaluation(statep)) == 100;

	(void) statep;
	//return won(statep) || drawn(statep);
	return true;
}

