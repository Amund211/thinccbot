#ifndef STATES_H_INCLUDED
#define STATES_H_INCLUDED

struct Action
{
	bool put; // Whether or not to put
};
struct Gamestate
{
	bool whitetoMove;
	unsigned int white;
	unsigned int timeSinceWhite;
	unsigned int black;
	unsigned int timeSinceBlack;
};

#endif
