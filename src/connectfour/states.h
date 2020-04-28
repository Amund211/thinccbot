#ifndef STATES_H_INCLUDED
#define STATES_H_INCLUDED

struct Action
{
	unsigned int column;
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
};

std::string aToString(Action const* actionp);
std::string sToString(Gamestate const* statep);

#endif
