#include "trees.h"

#include <string>
#include <cmath>
#include <algorithm>

#include "game.h"

void genChildren(Node* nodep)
{
	if (nodep->children) {
		// Node already has children
		return;
	}

	std::vector<Gamestate*> states;
	std::vector<Action*> actions;

	// Get list of legal moves
	unsigned int amtActions = getActions(nodep->state, states, actions);

	// Terminal state
	if (amtActions == 0) {
		nodep->amtChildren = 0;
		return;
	}

	// Allocate children array for node
	nodep->amtChildren = amtActions;
	nodep->children = new Node*[amtActions];

	for (unsigned int i=0; i<amtActions; i++) {
		// Append child node to parent
		nodep->children[i] = new Node{states[i], actions[i], 0, nullptr};
	}
}

void freeSubtree(Node* nodep)
{
	deleteState(nodep->state);
	deleteAction(nodep->action);
	for (unsigned int i=0; i<nodep->amtChildren; i++)
		freeSubtree(nodep->children[i]);
	delete nodep->children;
	delete nodep;
}

Evaluation bestAction(Node* nodep, unsigned int depth)
{
	// State must not be terminal
	float value;
	float childValue;
	Action* bestAction;

	// Generate children of root node
	genChildren(nodep);

	value = -INFINITY;
	for (unsigned int i=0; i<nodep->amtChildren; i++) {
		childValue = -negamax(nodep->children[i], depth - 1, -INFINITY, INFINITY);
		if (childValue > value) {
			// New best action
			value = childValue;
			bestAction = nodep->children[i]->action;
		}
	}
	return { bestAction, value };
}

float negamax(Node* nodep, unsigned int depth, float alpha, float beta)
{
	// Generate children of node
	genChildren(nodep);

	if (depth == 0 || nodep->amtChildren == 0) {
		// Gamestates must store `bool whiteToMove` as their first member
		return (*reinterpret_cast<bool*>(nodep->state) ? 1 : -1 ) * evaluation(nodep->state);
	}
	float value = -INFINITY;
	for (unsigned int i=0; i<nodep->amtChildren; i++) {
		value = std::max(value, -negamax(nodep->children[i], depth - 1, -beta, -alpha));
		alpha = std::max(alpha, value);
		if (alpha >= beta) {
			// TODO: Clean up remaining children
			break;
		}
	}
	return value;
}

