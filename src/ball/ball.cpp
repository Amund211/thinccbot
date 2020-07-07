#include <string>
#include <vector>

#include "states.h"
#include "../game.h"

/*
 * In this game two players take turns putting balls on a board
 * the first person to reach 9 or more balls wins.
 * If the player did not put a ball their last turn, they can
 * instead put 3 balls.
 * This is used as a simple example to test the tree-search.
 */

void deleteState(Gamestate* statep)
{
	delete statep;
}

void deleteAction(Action* actionp)
{
	delete actionp;
}

unsigned int getActions(Gamestate const* statep, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions) {
	// Game has ended
	if (statep->white >= 9) {
		return 0;
	} else if (statep->black >= 9) {
		return 0;
	}


	// Player does put
	actions.push_back(new Action{ true });
	Gamestate* state2 = new Gamestate{*statep};
	gamestates.push_back(state2);

	// Update player score
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
	state2->whitetoMove = !state2->whitetoMove;

	// Player does not put
	actions.push_back(new Action{ false });
	Gamestate* state1 = new Gamestate{*statep};
	gamestates.push_back(state1);

	// Update time since player last put
	if (state1->whitetoMove) {
		state1->timeSinceWhite++;
	} else {
		state1->timeSinceBlack++;
	}
	state1->whitetoMove = !state1->whitetoMove;

	return 2;
}

float evaluation(Gamestate const* statep) {
	if (statep->white >= 9) {
		return 100;
	} else if (statep->black >= 9) {
		return -100;
	}
	return static_cast<float>(static_cast<int>(statep->white) - static_cast<int>(statep->black));
}

std::string aToString(Action const* actionp) {
	return (actionp->put ? "put" : "no put");
}

std::string sToString(Gamestate const* statep) {
	return std::string("To move: ") + (statep->whitetoMove ? "white\n" : "black\n") +
		"\twhite: " + std::to_string(statep->white) + ":" + std::to_string(statep->timeSinceWhite) + "\n" +
		"\tblack: " + std::to_string(statep->black) + ":" + std::to_string(statep->timeSinceBlack);
}
