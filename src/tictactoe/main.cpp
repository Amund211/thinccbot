#include <iostream>
#include <string>

#include "states.h"
#include "../trees.h"
#include "../game.h"

bool gameOver(Gamestate const*);

int main() {
	Gamestate* sp = new Gamestate{true, {0, 0, 0, 0, 0, 0, 0, 0, 0}};

	std::cerr << "Current state:" << std::endl << sp->toString() << "\n" << std::endl;
	std::cerr << evaluation(sp);

	unsigned int depth = 15;
	bool player = true;

	Evaluation e = bestAction(sp, depth);
	std::cout <<
		"Best action is:\t" << e.action->toString() << std::endl <<
		"Evaluation:\t" << e.evaluation << std::endl;

	delete e.action;

	std::vector<Gamestate*> states;
	std::vector<Action*> actions;

	std::cout << sp->toString();
	while (!gameOver(sp)) {
		genChildren(sp, states, actions);

		unsigned int x, y;

		if (player && sp->xToMove) {
			// No input-validation
			std::cin >> x >> y;
		} else {
			e = bestAction(sp, depth);
			x = e.action->x;
			y = e.action->y;
		}

		// Free old state
		delete sp;

		// Find the corresponding child-node
		for (unsigned int i=0; i<actions.size(); i++) {
			if (actions[i]->x == x && actions[i]->y == y) {
				sp = states[i];
			} else {
				delete states[i];
			}
		}

		actions.clear();
		states.clear();

		if (!player || sp->xToMove)
			delete e.action;


		std::cout << sp->toString();
	}
}
