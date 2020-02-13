// Must be included before trees.h
#include "states.h"

#include "trees.h"

#include <string>
#include <cmath>
#include <algorithm>

#include "list.h"
#include "game.h"

void genChildren(Node *nodep) {
	// TODO: make these lists of pointers to avoid large copies
	// when they are transferred to the node
	List<Gamestate> states {};
	List<Action> actions {};

	// Get list of legal moves
	unsigned int amtActions = getActions(nodep->state, states, actions);

	// Terminal state
	if (amtActions == 0) {
		nodep->amtChildren = 0;
		return;
	}

	// Get initial state and action
	ListNode<Gamestate> *stateListnode = states.head;
	ListNode<Action> *actionListnode = actions.head;
	
	// Allocate children array for node
	nodep->amtChildren = amtActions;
	nodep->children = new Node*[amtActions];

	for (unsigned int i=0; i<amtActions; i++) {
		// Create new child node
		Node *childp = new Node;
		childp->state = stateListnode->data;
		childp->action = actionListnode->data;

		// Get next state and action
		stateListnode = stateListnode->next;
		actionListnode = actionListnode->next;

		// Append child node to parent
		nodep->children[i] = childp;
	}
}

Evaluation bestAction(Node *nodep, unsigned int depth) {
	// State must not be terminal
	float value;
	float childValue;
	Action bestAction;

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

float negamax(Node *nodep, unsigned int depth, float alpha, float beta) {
	// Generate children of node
	genChildren(nodep);

	if (depth == 0 || nodep->amtChildren == 0) {
		return (nodep->state.whitetoMove ? 1 : -1 ) * evaluation(nodep->state);
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

