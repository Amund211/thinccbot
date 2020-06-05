#include <cmath>
#include <map>
#include <memory>
#include <array>

#include "states.h"
#include "../game.h"

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

	bool contains(Coordinate pos)
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

enum class AttackStatus
{
	CLEAR,
	ATTACKED,
	PINNED
};

Gamestate* createState(Gamestate const* current, std::vector<Gamestate*>& gamestates)
{
	// Insert a copy of the current state into gamestates
	Gamestate* newstatep = new Gamestate{*current};
	gamestates.push_back(newstatep);

	// Update toMove
	newstatep->whiteToMove = !newstatep->whiteToMove;

	// Update en passant
	newstatep->passantSquare = {-1, -1};

	// Update ply since capture/pawn move
	newstatep->rule50Ply++;

	return newstatep;
}

void genPawnMoves(Gamestate const* statep, Color c, Coordinate pos, std::unique_ptr<Coordinate>& mustKill, std::unique_ptr<Line>& mustBlock, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions)
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
			if (mustKill && statep->passantSquare != *mustKill)
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

void genKnightMoves(Gamestate const* statep, Color c, Coordinate pos, std::unique_ptr<Coordinate>& mustKill, std::unique_ptr<Line>& mustBlock, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions)
{
	for (Coordinate offset : knightMoves) {
		Coordinate target = pos + offset;
		if (!target.isValid())
			continue;
		if (mustKill && target != *mustKill)
			continue;
		if (mustBlock && !mustBlock->contains(target))
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

void genKingMoves(Gamestate const* statep, Color c, const Coordinate& pos, std::array<bool, 8> attackedSquares, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions)
{
	for (auto it = kingMoveOrder.begin(); it != kingMoveOrder.end(); it++) {
		Coordinate target = pos + it->first;
		if (!target.isValid())
			continue;

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

AttackStatus checkAttack(const Board& b, const Coordinate& start, const Coordinate& end, Color opponent, Coordinate& pinnedPos)
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

void drawBoard(const Board&, Color, bool);

bool gameOver(Gamestate const* statep);
unsigned int getActions(Gamestate const* statep, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions) {
	// Both in stalemate and in mate no moves should be generated...
	// Then the check for stalemate vs mate is a simple as seeing whether the king is in check
	if (gameOver(statep)) {
		return 0;
	}

	Color toMove = statep->whiteToMove ? WHITE : BLACK;
	Color opponent = opponentColor(toMove);
	int opponentDirection = pawnDirection(opponent);

	// Bool array for the squares around the king
	// Ordered according to kingMoveOrder
	std::array<bool, 8> kingNeighborAttacked = {false, false, false, false, false, false, false, false};

	// If there is an associated object, all valid moves must kill the piece at that pos
	std::unique_ptr<Coordinate> mustKill;

	// If there is an associated object, all valid moves must move to that line
	std::unique_ptr<Line> mustBlock;

	// Map from pinned postions to the line they are pinned on
	std::map<Coordinate, Line> pinnedPositions;

	// Amount of checks on the king
	unsigned int amtChecks = 0;

	Coordinate kingPos = findKing(statep->board, toMove);

	// Passed into checkAttack to return the position of the pinnedPiece
	Coordinate pinnedPos;

	for (int rank=0; rank<8; rank++) {
		if (amtChecks >= 2)
			break;

		for (int file=0; file<8; file++) {
			if (amtChecks >= 2)
				break;

			Piece p = statep->board.get({rank, file});
			if (p == NONE || pieceColor(p) == toMove)
				// No piece or our piece
				continue;

			Delta delta = kingPos - Coordinate{rank, file};
			switch (pieceType(p)) {
				case PAWN:
					if (delta.rank == opponentDirection && std::abs(delta.file) == 1) {
						amtChecks++;
						if (!mustKill)
							mustKill = std::make_unique<Coordinate>(rank, file);
					} else {
						// Check if the pawn is guarding any squares next to the king
						int relativeAttackRank = delta.rank - opponentDirection;
						int relativeAttackCenterFile = delta.file;
						if (std::abs(relativeAttackRank) > 1 || std::abs(relativeAttackCenterFile) > 2)
							continue;
						for (int fileOffset=-1; fileOffset<=1; fileOffset+=2) {
							auto it = kingMoveOrder.find({-relativeAttackRank, -relativeAttackCenterFile + fileOffset});
							if (it != kingMoveOrder.end()) {
								kingNeighborAttacked[it->second] = true;
							}
						}
					}
					break;
				case KNIGHT:
					// Equivalient with valid knight-move when
					// rank and file are integers
					if (std::abs(delta.rank * delta.file) == 2) {
						amtChecks++;
						if (!mustKill)
							mustKill = std::make_unique<Coordinate>(rank, file);
					} else {
						// Check if the knight is guarding any squares next to the king
						for (auto it = kingMoveOrder.begin(); it != kingMoveOrder.end(); it++) {
							Delta guardTarget = delta + it->first;
							if (std::abs(guardTarget.rank * guardTarget.file) == 2) {
								kingNeighborAttacked[it->second] = true;
							}
						}
					}
					break;
				case BISHOP:
				case ROOK:
				case QUEEN:
					// New scope to not initialize any new variables in the switch
					{
						bool diag = delta.isDiagonal();
						bool straight = delta.isStraight();
						if (!straight && !diag)
							continue;
						if (pieceType(p) == BISHOP && !diag)
							continue;
						if (pieceType(p) == ROOK && !straight)
							continue;

						AttackStatus status = checkAttack(statep->board, kingPos, {rank, file}, opponent, pinnedPos);
						switch (status) {
							case AttackStatus::CLEAR:
								break;
							case AttackStatus::ATTACKED:
								amtChecks++;
								if (!mustBlock)
									mustBlock = std::make_unique<Line>(kingPos, Coordinate{rank, file});

								// Remove kingmove
								kingNeighborAttacked[kingMoveOrder.at(-delta.step())] = true;
								break;
							case AttackStatus::PINNED:
								pinnedPositions.emplace(pinnedPos, Line{kingPos, {rank, file}});
								break;
						}
					}
					break;
				case KING:
					// A king can't check, but can block squares
					// LEGAWL KINGMOVE = INFNORM = 1
					if (std::abs(delta.rank) > 2 || std::abs(delta.file) > 2)
						continue;
					for (auto it = kingMoveOrder.begin(); it != kingMoveOrder.end(); it++) {
						Delta guardTarget = delta + it->first;
						if (guardTarget.infNorm() == 1) {
							kingNeighborAttacked[it->second] = true;
						}
					}
					break;
				default:
					break;
			}
		}
	}

	// 2+ attackers -> only king-moves can get out of check
	if (amtChecks >= 2) {
		genKingMoves(statep, toMove, kingPos, kingNeighborAttacked, gamestates, actions);
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
						genPawnMoves(statep, toMove, {rank, file}, mustKill, mustBlock, gamestates, actions);
						break;
					case KNIGHT:
						genKnightMoves(statep, toMove, {rank, file}, mustKill, mustBlock, gamestates, actions);
						break;
					case BISHOP:
						break;
					case ROOK:
						break;
					case QUEEN:
						break;
					case KING:
						//if (!mustKill && !mustBlock)
							genKingMoves(statep, toMove, kingPos, kingNeighborAttacked, gamestates, actions);
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

