// Must be included before trees.h
#include "states.h"

#include "trees.h"

#include <string>
#include <cmath>
#include <algorithm>

#include "list.h"
#include "game.h"

void genChildren(Node *nodep) {
	List<Gamestate*> *stateps = new List<Gamestate*>;
	List<Action> *actions = new List<Action>;

	// Get list of legal moves
	unsigned int amtActions = getActions(nodep->statep, stateps, actions);

	// Terminal state
	if (amtActions == 0) {
		nodep->amtChildren = 0;
		return;
	}

	// Get initial state and action
	ListNode<Gamestate*> *stateListnode = stateps->head;
	ListNode<Action> *actionListnode = actions->head;
	
	// Allocate children array for node
	nodep->amtChildren = amtActions;
	nodep->children = new Node*[amtActions];

	for (unsigned int i=0; i<amtActions; i++) {
		// Create new child node
		Node *childp = new Node;
		childp->statep = stateListnode->data;
		childp->action = actionListnode->data;

		// Get next state and action
		stateListnode = stateListnode->next;
		actionListnode = actionListnode->next;

		// Append child node to parent
		nodep->children[i] = childp;
	}
}

Action bestAction(Node *nodep, unsigned int depth) {
	// State must not be terminal
	float value;
	float childValue;
	Action bestAction;

	// Generate children of root node
	genChildren(nodep);

	value = -INFINITY;
	for (unsigned int i=0; i<nodep->amtChildren; i++) {
		childValue = -negamax(nodep->children[i], depth - 1);
		if (childValue > value) {
			// New best action
			value = childValue;
			bestAction = nodep->children[i]->action;
		}
	}
	return bestAction;
}

float negamax(Node *nodep, unsigned int depth) {
	// Generate children of node
	genChildren(nodep);

	if (depth == 0 || nodep->amtChildren == 0) {
		return (nodep->statep->whitetoMove ? 1 : -1 ) * evaluation(nodep->statep);
	}
	float value;
	value = -INFINITY;
	for (unsigned int i=0; i<nodep->amtChildren; i++) {
		value = std::max(value, -negamax(nodep->children[i], depth - 1));
	}
	return value;
}

