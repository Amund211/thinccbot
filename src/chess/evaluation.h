#ifndef EVALUATION_H_INCLUDED
#define EVALUATION_H_INCLUDED

#include "../game.h"

#include "states.h"

// PSQT
#include "psqt.h"

// float evaluation(...) is declared in ../game.h

bool gameOver(Gamestate const* statep);

inline constexpr float materialValue[7] = {0, 1, 3, 3, 5, 9, 0};

float materialCount(const Board& b);

float psqtScore(const Board& b);

float evaluation(Gamestate const* statep);

#endif
