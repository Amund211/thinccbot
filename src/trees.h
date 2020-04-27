#ifndef TREES_H_INCLUDED
#define TREES_H_INCLUDED

#include "game.h"

struct Node
{
	Gamestate* state;
	Action* action;
	unsigned int amtChildren;
	Node** children; // Array of pointers to children
	~Node();
};

struct Evaluation
{
	Action* action;
	float evaluation;
};

void genChildren(Node* nodep);

void freeSubtree(Node* nodep);

Evaluation bestAction(Node* nodep, unsigned int depth);
float negamax(Node* nodep, unsigned int depth, float alpha, float beta);

#endif
