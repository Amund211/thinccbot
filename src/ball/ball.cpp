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

unsigned int getActions(const Gamestate &state, List<Gamestate> &gamestates, List<Action> &actions) {
	// Game has ended
	if (state.white >= 9) {
		return 0;
	} else if (state.black >= 9) {
		return 0;
	}


	// Player does not put
	actions.add({ false });
	Gamestate &state1 = gamestates.add(state);

	// Update time since player last put
	if (state1.whitetoMove) {
		state1.timeSinceWhite++;
	} else {
		state1.timeSinceBlack++;
	}
	state1.whitetoMove = !state.whitetoMove;

	// Player does put
	actions.add({ true });
	Gamestate &state2 = gamestates.add(state);

	// Update player score
	if (state2.whitetoMove) {
		if (state2.timeSinceWhite >= 1) {
			state2.white += 3;
			state2.timeSinceWhite = 0;
		} else {
			state2.white++;
		}
	} else {
		if (state2.timeSinceBlack >= 1) {
			state2.black += 3;
			state2.timeSinceBlack = 0;
		} else {
			state2.black++;
		}
	}
	state2.whitetoMove = !state.whitetoMove;

	return 2;
}

float evaluation(const Gamestate &state) {
	if (state.white >= 9) {
		return 100;
	} else if (state.black >= 9) {
		return -100;
	}
	return static_cast<float>(static_cast<int>(state.white) - static_cast<int>(state.black));
}

std::string aToString(const Action &action) {
	return (action.put ? "put" : "no put");
}

std::string sToString(const Gamestate &state) {
	return std::string("To move: ") + (state.whitetoMove ? "white\n" : "black\n") +
		"\twhite: " + std::to_string(state.white) + ":" + std::to_string(state.timeSinceWhite) + "\n" +
		"\tblack: " + std::to_string(state.black) + ":" + std::to_string(state.timeSinceBlack);
}
