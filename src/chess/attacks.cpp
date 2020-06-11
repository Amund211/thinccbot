#include "attacks.h"

#include "pieces.h"
#include "board.h"

#include <array>
#include <memory>
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

unsigned int getAttacks(
	// Parameters
	const Board& board,
	const Coordinate& kingPos,
	Color toMove,
	// Output
	std::array<bool, 8>& attackedSquares,
	std::unique_ptr<Coordinate>& mustKill,
	std::unique_ptr<Line>& mustBlock,
	std::map<Coordinate, Line>& pinnedPositions
)
{
	// Returns amount of checks on the king

	Color opponent = opponentColor(toMove);
	int opponentDirection = pawnDirection(opponent);

	// Amount of checks on the king
	unsigned int amtChecks = 0;

	// Passed into checkAttack to return the position of the pinnedPiece
	Coordinate pinnedPos;

	for (int rank=0; rank<8; rank++) {
		for (int file=0; file<8; file++) {
			Piece p = board.get({rank, file});
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
							AttackStatus status = checkAttack(board, kingPos, {rank, file}, opponent, pinnedPos);
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
									checkGuarded(board, {kingPos.rank + 1, file}, kingPos, {rank, file}, attackedSquares);

								if (delta.rank <= 0)
									checkGuarded(board, {kingPos.rank - 1, file}, kingPos, {rank, file}, attackedSquares);
							}

							if (std::abs(delta.rank) == 1) {
								if (delta.file >= 0)
									checkGuarded(board, {rank, kingPos.file + 1}, kingPos, {rank, file}, attackedSquares);

								if (delta.file <= 0)
									checkGuarded(board, {rank, kingPos.file - 1}, kingPos, {rank, file}, attackedSquares);
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
									checkGuarded(board, kingPos + Coordinate{-downwardSign * position, -downwardSign * !position}, kingPos, {rank, file}, attackedSquares);
									break;
								case 2:
									// Target is the square diagonal to the king and in line with the attacker
									checkGuarded(board, kingPos + Coordinate{upwardSign, -upwardSign}, kingPos, {rank, file}, attackedSquares);
									break;
								default:
									// Line is too far away
									break;
							}

							switch (std::abs(downwardOffset)) {
								case 1:
									// Target is the furthest of the two squares next to the king and in line with the attacker
									checkGuarded(board, kingPos + Coordinate{downwardSign * position, downwardSign * !position}, kingPos, {rank, file}, attackedSquares);
									break;
								case 2:
									// Target is the square diagonal to the king and in line with the attacker
									checkGuarded(board, kingPos + Coordinate{downwardSign, downwardSign}, kingPos, {rank, file}, attackedSquares);
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

	return amtChecks;
}
