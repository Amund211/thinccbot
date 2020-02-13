// Users must include "states.h"

#ifndef TREES_H_INCLUDED
#define TREES_H_INCLUDED

struct Node
{
	Gamestate state;
	Action action;
	unsigned int amtChildren;
	Node **children; // Array of pointers to children
};

struct Evaluation
{
	Action action;
	float evaluation;
};

void genChildren(Node *nodep);

Evaluation bestAction(Node *nodep, unsigned int depth);
float negamax(Node *nodep, unsigned int depth, float alpha, float beta);

#endif
