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
	Color opponent = opponentColor(toMove);
	int opponentDirection = pawnDirection(opponent);

	// Bool array for the squares around the king
	// Ordered according to kingMoveOrder
	std::array<bool, 8> attackedSquares = {
		false, false, false, false, false, false, false, false
	};

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
		for (int file=0; file<8; file++) {
			Piece p = statep->board.get({rank, file});
			if (p == NONE || pieceColor(p) == toMove)
				// No piece or our piece
				continue;

			Delta delta = kingPos - Coordinate{rank, file};
			switch (pieceType(p)) {
				case PAWN:
					if (delta.rank == opponentDirection && std::abs(delta.file) == 1) {
						if (!amtChecks)
							mustKill = std::make_unique<Coordinate>(rank, file);
						amtChecks++;
					} else {
						// Check if the pawn is guarding any squares next to the king
						int relativeAttackRank = delta.rank - opponentDirection;
						int relativeAttackCenterFile = delta.file;
						if (std::abs(relativeAttackRank) > 1 || std::abs(relativeAttackCenterFile) > 2)
							continue;
						for (int fileOffset=-1; fileOffset<=1; fileOffset+=2) {
							auto it = kingMoveOrder.find({-relativeAttackRank, -relativeAttackCenterFile + fileOffset});
							if (it != kingMoveOrder.end()) {
								attackedSquares[it->second] = true;
							}
						}
					}
					break;
				case KNIGHT:
					// Equivalient with valid knight-move when
					// rank and file are integers
					if (std::abs(delta.rank * delta.file) == 2) {
						if (!amtChecks)
							mustKill = std::make_unique<Coordinate>(rank, file);
						amtChecks++;
					} else {
						// Check if the knight is guarding any squares next to the king
						for (auto it = kingMoveOrder.begin(); it != kingMoveOrder.end(); it++) {
							Delta guardTarget = delta + it->first;
							if (std::abs(guardTarget.rank * guardTarget.file) == 2) {
								attackedSquares[it->second] = true;
							}
						}
					}
					break;
				case BISHOP:
				case ROOK:
				case QUEEN:
					// New scope to not initialize any new variables in the switch
					{
						PieceType type = pieceType(p);

						bool diag = delta.isDiagonal();
						bool straight = delta.isStraight();

						bool moveDiag = (type == BISHOP) || (type == QUEEN);
						bool moveStraight = (type == ROOK) || (type == QUEEN);

						bool isLinedUp = (diag && moveDiag) || (straight && moveStraight);

						if (isLinedUp) {
							AttackStatus status = checkAttack(statep->board, kingPos, {rank, file}, opponent, pinnedPos);
							switch (status) {
								case AttackStatus::CLEAR:
									break;
								case AttackStatus::ATTACKED:
									if (!amtChecks)
										mustBlock = std::make_unique<Line>(kingPos, Coordinate{rank, file});
									amtChecks++;

									{
										Delta s {delta.step()};
										// King can't move away from this attacker
										attackedSquares[kingMoveOrder.at(s)] = true;

										if (rank + s.rank != kingPos.rank || file + s.file != kingPos.file)
											// King can't move towards from his attacker if he can't capture it
											attackedSquares[kingMoveOrder.at(-s)] = true;
									}
									break;
								case AttackStatus::PINNED:
									pinnedPositions.emplace(pinnedPos, Line{kingPos, {rank, file}});
									break;
							}
						}

						// See if it blocks any squares next to the king
						if (moveStraight) {
							if (std::abs(delta.file) == 1) {
								if (delta.rank >= 0)
									checkGuarded(statep->board, {kingPos.rank + 1, file}, kingPos, {rank, file}, attackedSquares);

								if (delta.rank <= 0)
									checkGuarded(statep->board, {kingPos.rank - 1, file}, kingPos, {rank, file}, attackedSquares);
							}

							if (std::abs(delta.rank) == 1) {
								if (delta.file >= 0)
									checkGuarded(statep->board, {rank, kingPos.file + 1}, kingPos, {rank, file}, attackedSquares);

								if (delta.file <= 0)
									checkGuarded(statep->board, {rank, kingPos.file - 1}, kingPos, {rank, file}, attackedSquares);
							}
						}

						// See if it blocks any squares next to the king
						if (moveDiag && !diag) {
							// Rank offset for the diagonal lines going through the attacking piece
							// +file, +rank diagonal
							int upwardOffset = delta.file - delta.rank;
							// +file, -rank diagonal
							int downwardOffset = -delta.file - delta.rank;

							// The attacking line lies above the upward line through the king
							bool aboveUpward = upwardOffset > 0;
							// The attacking line lies above the downward line through the king
							bool aboveDownward = downwardOffset > 0;

							bool position = aboveUpward != aboveDownward;
							int upwardSign = aboveUpward ? 1 : -1;
							int downwardSign = aboveDownward ? 1 : -1;

							// Lines guarding kingmoves have an offset of +-1 or +-2
							// Lines with an offset of 0 are handled earlier by if (isLinedUp)
							switch (std::abs(upwardOffset)) {
								case 1:
									// Target is the furthest of the two squares next to the king and in line with the attacker
									checkGuarded(statep->board, kingPos + Coordinate{-downwardSign * position, -downwardSign * !position}, kingPos, {rank, file}, attackedSquares);
									break;
								case 2:
									// Target is the square diagonal to the king and in line with the attacker
									checkGuarded(statep->board, kingPos + Coordinate{upwardSign, -upwardSign}, kingPos, {rank, file}, attackedSquares);
									break;
								default:
									// Line is too far away
									break;
							}

							switch (std::abs(downwardOffset)) {
								case 1:
									// Target is the furthest of the two squares next to the king and in line with the attacker
									checkGuarded(statep->board, kingPos + Coordinate{downwardSign * position, downwardSign * !position}, kingPos, {rank, file}, attackedSquares);
									break;
								case 2:
									// Target is the square diagonal to the king and in line with the attacker
									checkGuarded(statep->board, kingPos + Coordinate{downwardSign, downwardSign}, kingPos, {rank, file}, attackedSquares);
									break;
								default:
									// Line is too far away
									break;
							}
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
							attackedSquares[it->second] = true;
						}
					}
					break;
				default:
					throw std::invalid_argument("Invalid piece on board");
					break;
			}
		}
	}

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
