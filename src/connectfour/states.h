#ifndef STATES_H_INCLUDED
#define STATES_H_INCLUDED

#include <string>

struct Action
{
	unsigned int column;

	std::string toString();
};

struct Column
{
	int8_t stack[6]; // yellow = 1, blank = 0, red = -1

	bool isFull() const;
	void drop(int8_t color);
};

struct Gamestate
{
	bool yellowToMove;
	Column columns[7];

	Gamestate(bool yellowToMove=true);
	Gamestate(bool yellowToMove, Column columns[7]);

	std::string toString();
};

#endif
