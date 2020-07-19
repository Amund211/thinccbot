#ifndef TREES_H_INCLUDED
#define TREES_H_INCLUDED

#include "game.h"

struct Evaluation
{
	Action* action;
	float evaluation;
};

void genChildren(Gamestate const* statep, std::vector<Gamestate*>& states, std::vector<Action*>& actions);

Evaluation bestAction(Gamestate const* statep, unsigned int depth);
float negamax(Gamestate const* statep, unsigned int depth, float alpha, float beta);

#endif
