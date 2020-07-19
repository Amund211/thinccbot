#include "trees.h"

#include <string>
#include <cmath>
#include <algorithm>

#include "game.h"

Evaluation bestAction(Gamestate const* statep, unsigned int depth)
{
	// State must not be terminal
	float value;
	float childValue;
	Action* bestAction = nullptr;

	std::vector<Gamestate*> states;
	std::vector<Action*> actions;

	// Generate children of root node
	genChildren(statep, states, actions);

	value = -INFINITY;
	for (unsigned int i=0; i<states.size(); i++) {
		childValue = -negamax(states[i], depth - 1, -INFINITY, INFINITY);
		if (childValue > value) {
			// New best action
			value = childValue;
			if (bestAction) {
				deleteAction(bestAction);
			}
			bestAction = actions[i];
		} else {
			deleteAction(actions[i]);
		}
		deleteState(states[i]);
	}
	return { bestAction, value };
}

float negamax(Gamestate const* statep, unsigned int depth, float alpha, float beta)
{
	std::vector<Gamestate*> states;
	std::vector<Action*> actions;

	if (depth == 0 || (genChildren(statep, states, actions), states.size() == 0)) {
		float eval = evaluation(statep);
		int sign = *reinterpret_cast<bool const*>(statep) ? 1 : -1;
		// Weigh won/lost nodes by distance to emulate human play
		// Gamestates must store `bool whiteToMove` as their first member
		if (eval == 100)
			return sign * (eval + depth);
		else if (eval == -100)
			return sign * (eval - depth);
		else
			return sign * eval;
	}

	for (Action* nextAction : actions)
		deleteAction(nextAction);

	float value = -INFINITY;
	for (Gamestate* nextState : states) {
		if (alpha < beta) {
			value = std::max(value, -negamax(nextState, depth - 1, -beta, -alpha));
			alpha = std::max(alpha, value);
		}
		deleteState(nextState);
	}
	return value;
}

