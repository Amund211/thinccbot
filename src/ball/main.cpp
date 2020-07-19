#include <iostream>
#include <string>

#include "states.h"
#include "../trees.h"
#include "../game.h"


int main() {
	Gamestate* sp = new Gamestate{ true, 0, 0, 0, 0 };
	std::cout << "Current state:" << std::endl << sp->toString() << "\n" << std::endl;

	unsigned int depth = 15;

	Evaluation e = bestAction(sp, depth);
	std::cout <<
		"Best action is:\t" << e.action->toString() << std::endl <<
		"Evaluation:\t" << e.evaluation << std::endl;
}
