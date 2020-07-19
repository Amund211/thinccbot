#ifndef STATES_H_INCLUDED
#define STATES_H_INCLUDED

#include <string>

struct Action
{
	bool put; // Whether or not to put

	std::string toString();
};

struct Gamestate
{
	bool whitetoMove;
	unsigned int white;
	unsigned int timeSinceWhite;
	unsigned int black;
	unsigned int timeSinceBlack;

	std::string toString();
};

#endif
