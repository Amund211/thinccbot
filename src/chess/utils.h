#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "pieces.h"
#include "states.h"

#include <string>

extern const std::string STARTING_FEN;

Coordinate SAN2Coord(std::string SAN);

void drawBoard(const Board& b, Color perspective=WHITE, bool colorTerminal=false);

#endif

