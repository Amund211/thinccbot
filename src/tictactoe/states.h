#ifndef STATES_H_INCLUDED
#define STATES_H_INCLUDED

struct Action
{
	unsigned int x, y;
};

struct Gamestate
{
	bool xToMove;
	int8_t board[9]; // X = 1, blank = 0, O = -1
};

std::string aToString(Action const* actionp);
std::string sToString(Gamestate const* statep);

#endif
