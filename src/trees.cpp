// Must be included before trees.h
#include "states.h"

#include "trees.h"

#include <string>
#include <cmath>
#include <algorithm>

#include "list.h"
#include "game.h"

void genTree(Node *nodep, unsigned int depth) {
	if (depth == 0)
		return;

	List<Gamestate*> *stateps = new List<Gamestate*>;
	List<Action> *actions = new List<Action>;

	// Get list of legal moves
	unsigned int amtActions = getActions(nodep->statep, stateps, actions);

	// Terminal state
	if (amtActions == 0)
		return;

	// Get initial state and action
	ListNode<Gamestate*> *stateListnode = stateps->head;
	ListNode<Action> *actionListnode = actions->head;
	
	// Allocate children array for node
	nodep->amtChildren = amtActions;
	nodep->children = new Node*[amtActions];

	for (int i=0; i<amtActions; i++) {
		// Create new child node
		Node *childp = new Node;
		childp->statep = stateListnode->data;
		childp->action = actionListnode->data;

		// Get next state and action
		stateListnode = stateListnode->next;
		actionListnode = actionListnode->next;

		// Append child node to parent
		nodep->children[i] = childp;
		genTree(childp, depth - 1);
	}
}

Action bestAction(Node *nodep, unsigned int depth) {
	// Depth >= 1
	// Maybe maximizing by default???
	float value;
	float childValue;
	const bool maximizing = nodep->statep->whitetoMove;
	Action bestAction;
	if (maximizing) {
		value = -INFINITY;
		for (unsigned int i=0; i<nodep->amtChildren; i++) {
			childValue = minimax(nodep->children[i], depth - 1);
			if (childValue > value) {
				// New best action
				value = childValue;
				bestAction = nodep->children[i]->action;
			}
		}
		return bestAction;
	} else {
		value = INFINITY;
		for (unsigned int i=0; i<nodep->amtChildren; i++) {
			childValue = minimax(nodep->children[i], depth - 1);
			if (childValue > value) {
				// New best action
				value = childValue;
				bestAction = nodep->children[i]->action;
			}
		}
		return bestAction;
	}
}

float minimax(Node *nodep, unsigned int depth) {
	if (depth == 0 || nodep->amtChildren == 0) {
		return evaluation(nodep->statep);
	}
	// float value = -1 * std::numeric_limits<float>::infinity();
	const bool maximizing = nodep->statep->whitetoMove;
	float value;
	if (maximizing) {
		value = -INFINITY;
		for (unsigned int i=0; i<nodep->amtChildren; i++) {
			value = std::max(value, minimax(nodep->children[i], depth - 1));
		}
		return value;
	} else {
		value = INFINITY;
		for (unsigned int i=0; i<nodep->amtChildren; i++) {
			value = std::min(value, minimax(nodep->children[i], depth - 1));
		}
		return value;
	}
}

