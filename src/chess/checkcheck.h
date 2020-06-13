#ifndef CHECKCHECK_H_INCLUDED
#define CHECKCHECK_H_INCLUDED

#include "states.h"

enum class GameStatus
{
	WIN,
	DRAW,
	UNDECIDED
};

GameStatus getGameStatus(Gamestate const* statep);

#endif
