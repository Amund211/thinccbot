#include <string>
#include <vector>
#include <algorithm>

#include "states.h"
#include "../game.h"

void deleteState(Gamestate* state) {
	delete state;
}

void deleteAction(Action* action) {
	delete action;
}

bool gameOver(Gamestate const* statep)
{
	if (evaluation(statep) != 0)
		return true;
	for (unsigned int i=0; i<9; i++)
		if (statep->board[i] == 0)
			return false;
	// No blank spaces
	return true;
}

unsigned int getActions(Gamestate const* statep, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions) {
	if (gameOver(statep)) {
		return 0;
	}

	for (unsigned int y=0; y<3; y++) {
		for (unsigned int x=0; x<3; x++) {
			if (statep->board[y*3 + x] == 0) {
				// Blank space -> placable
				actions.push_back(new Action{x, y});
				Gamestate* newstatep = new Gamestate{*statep};
				gamestates.push_back(newstatep);
				newstatep->board[y*3 + x] = statep->xToMove ? 1 : -1;
				newstatep->xToMove = !newstatep->xToMove;
			}
		}
	}

	// Place a winning move first
	for (unsigned int i=1; i<gamestates.size(); i++) {
		// Evaluation and player have the same sign -> win
		if (evaluation(gamestates[i]) * (!gamestates[i]->xToMove ? 1 : -1) > 0) {
			std::swap(gamestates[0], gamestates[i]);
			std::swap(actions[0], actions[i]);
			break;
		}
	}

	return actions.size();
}

int scoreLine(Gamestate const* statep, int x0, int y0, int xinc, int yinc)
{
	int score = 0;
	for (int inc=0; inc<3; inc++)
		score += statep->board[3*(y0+inc*yinc) + x0 + inc*xinc];

	if (score == 3)
		return 3;
	else if (score == -3)
		return -3;
	else
		return 0;
}

float evaluation(Gamestate const* statep) {
	// No heuristic, assumes the tree is searched to game end
	int score = 0;

	score += scoreLine(statep, 0, 0, 1, 0);
	score += scoreLine(statep, 0, 1, 1, 0);
	score += scoreLine(statep, 0, 2, 1, 0);

	score += scoreLine(statep, 0, 0, 0, 1);
	score += scoreLine(statep, 1, 0, 0, 1);
	score += scoreLine(statep, 2, 0, 0, 1);

	score += scoreLine(statep, 0, 0, 1, 1);
	score += scoreLine(statep, 0, 2, 1, -1);

	// Normalize the score to not prioritize double three in a row.
	// This makes it prioritize the moves in the order they are generated in getActions
	if (score > 0)
		return 100;
	else if (score < 0)
		return -100;
	else
		return 0;
}

std::string aToString(Action const* actionp) {
	return std::to_string(actionp->x) + ", " + std::to_string(actionp->y);
}

std::string sToString(Gamestate const* statep) {
	std::string s{"To move: "};
	s.append(statep->xToMove ? "X\n" : "O\n");
	for (unsigned int y=0; y<3; y++) {
		for (unsigned int x=0; x<3; x++) {
			if (statep->board[y*3 + x] == 0) {
				s.append(" ");
			} else {
				s.append(statep->board[y*3 + x] == 1 ? "X" : "O");
			}
		}
		s.append("\n");
	}
	return s;
}
