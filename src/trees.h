// Users must include "states.h"

#ifndef TREES_H_INCLUDED
#define TREES_H_INCLUDED

struct Node
{
	Gamestate *statep;
	Action action;
	unsigned int amtChildren;
	Node **children; // Array of pointers to children
};

void genTree(Node *nodep, unsigned int depth);

Action bestAction(Node *nodep, unsigned int depth);
float minimax(Node *nodep, unsigned int depth);

#endif
