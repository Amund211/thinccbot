#include "evaluation.h"

#include "states.h"
#include "pieces.h"
#include "../game.h"

constexpr float materialValue[7] = {0, 1, 3, 3, 5, 9, 0};

float materialCount(const Board& b)
{
	float count = 0;
	for (int rank=0; rank<8; rank++)
		for (int file=0; file<8; file++) {
			Piece tmpPiece = b.get({rank, file});
			count += pawnDirection(pieceColor(tmpPiece)) * materialValue[pieceType(tmpPiece)];
		}
	return count;
}

float evaluation(Gamestate const* statep) {
	if (statep->rule50Ply >= 100)
		// Both players can claim a draw
		return 0;

	return materialCount(statep->board);
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
