#ifndef STATES_H_INCLUDED
#define STATES_H_INCLUDED

#include <string>

struct Action
{
	unsigned int x, y;

	std::string toString();
};

struct Gamestate
{
	bool xToMove;
	int8_t board[9]; // X = 1, blank = 0, O = -1

	std::string toString();
};

#endif
