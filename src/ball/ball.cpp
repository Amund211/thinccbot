#include <string>

#include "../list.h"
#include "../states.h"
#include "../game.h"

/*
 * In this game two players take turns putting balls on a board
 * the first person to reach 9 or more balls wins.
 * If the player did not put a ball their last turn, they can
 * instead put 3 balls.
 * This is used as a simple example to test the tree-search.
 */

unsigned int getActions(Gamestate *statep, List<Gamestate*> *gamestateps, List<Action> *actions) {
	// Game has ended
	if (statep->white >= 9) {
		return 0;
	} else if (statep->black >= 9) {
		return 0;
	}

	// Add the two possible actions
	actions->add({ false });
	actions->add({ true });

	// Add the new possible gamestates
	Gamestate *state1 = new Gamestate;
	Gamestate *state2 = new Gamestate;

	// Shallow copy of gamestate
	*state1 = *statep;
	*state2 = *statep;

	// Update time since player last put
	if (state1->whitetoMove) {
		state1->timeSinceWhite++;
	} else {
		state1->timeSinceBlack++;
	}

	if (state2->whitetoMove) {
		if (state2->timeSinceWhite >= 1) {
			state2->white += 3;
			state2->timeSinceWhite = 0;
		} else {
			state2->white++;
		}
	} else {
		if (state2->timeSinceBlack >= 1) {
			state2->black += 3;
			state2->timeSinceBlack = 0;
		} else {
			state2->black++;
		}
	}

	// Set tomove
	state1->whitetoMove = !state1->whitetoMove;
	state2->whitetoMove = !state2->whitetoMove;


	gamestateps->add(state1);
	gamestateps->add(state2);

	return 2;
}

float evaluation(Gamestate *statep) {
	if (statep->white >= 9) {
		return 100;
	} else if (statep->black >= 9) {
		return -100;
	}
	return (float) ((int) statep->white - (int) statep->black);
}

std::string pprint(Action action) {
	return (action.put ? "put" : "no put");
}
