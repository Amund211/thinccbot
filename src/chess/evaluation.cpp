#include "evaluation.h"

#include "states.h"
#include "pieces.h"
#include "checkcheck.h"
#include "../game.h"

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

float psqtScore(const Board& b)
{
	float score = 0;
	for (int rank=0; rank<8; rank++)
		for (int file=0; file<8; file++)
			score += PSQT[b.get({rank, file})][rank][file];
	return score;
}

float evaluation(Gamestate const* statep) {
	GameStatus status = getGameStatus(statep);
	switch (status) {
		case GameStatus::WIN:
			return (statep->whiteToMove ? -100 : 100);
		case GameStatus::DRAW:
			return 0;
		case GameStatus::UNDECIDED:
			break;
	}

	if (statep->rule50Ply >= 100)
		// Both players can claim a draw
		return 0;

	return psqtScore(statep->board);
	//return materialCount(statep->board);
}

bool gameOver(Gamestate const* statep)
{
	return getGameStatus(statep) != GameStatus::UNDECIDED;
}
