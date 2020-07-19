#include <iostream>
#include <string>
#include <vector>

#include "states.h"
#include "../trees.h"
#include "../game.h"

bool gameOver(Gamestate const*);
float evaluation(Gamestate const*);

int main() {
	Gamestate* sp = new Gamestate();

	unsigned int depth = 9;
	bool player = true;
	unsigned int ply = 0;

	Evaluation e = bestAction(sp, depth);
	std::cout <<
		"Best action is:\t" << e.action->toString() << std::endl <<
		"Evaluation:\t" << e.evaluation << std::endl;

	delete e.action;

	std::cout << sp->toString();

	std::vector<Gamestate*> states;
	std::vector<Action*> actions;

	while (!gameOver(sp)) {
		genChildren(sp, states, actions);

		unsigned int column;

		std::cout << "Current state evaluation:\t" << evaluation(sp) << std::endl;
		if (player && sp->yellowToMove) {
			// No input-validation
			std::cin >> column;
		} else {
			e = bestAction(sp, depth);
			column = e.action->column;
			std::cout << "Node evaluation:\t" << e.evaluation << std::endl;
		}

		// Free old state
		delete sp;

		// Find the corresponding child-node
		for (unsigned int i=0; i<actions.size(); i++) {
			if (actions[i]->column == column) {
				sp = states[i];
			} else {
				delete states[i];
			}
		}

		states.clear();
		actions.clear();

		if (!player || sp->yellowToMove)
			delete e.action;

		std::cout << "\n" << sp->toString();
		std::cout << "Ply: " << ++ply << std::endl;
	}
	std::cout << e.evaluation << std::endl;
	std::cout << evaluation(sp) << std::endl;
}
