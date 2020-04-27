#include <iostream>
#include <string>

#include "states.h"
#include "../trees.h"
#include "../game.h"

bool gameOver(Gamestate const*);

int main() {
	Node *root = new Node;
	root->state = new Gamestate{true, {0, 0, 0, 0, 0, 0, 0, 0, 0}};

	std::cerr << "Current state:" << std::endl << sToString(root->state) << "\n" << std::endl;
	std::cerr << evaluation(root->state);

	unsigned int depth = 15;
	bool player = true;

	Evaluation e = bestAction(root, depth);
	std::cout <<
		"Best action is:\t" << aToString(e.action) << std::endl <<
		"Evaluation:\t" << e.evaluation << std::endl;

	std::cout << sToString(root->state);
	while (!gameOver(root->state)) {
		unsigned int x, y;

		if (player && root->state->xToMove) {
			// No input-validation
			std::cin >> x >> y;
		} else {
			e = bestAction(root, depth);
			x = e.action->x;
			y = e.action->y;
		}

		// Find the corresponding child-node
		for (unsigned int i=0; i<root->amtChildren; i++) {
			if (root->children[i]->action->x == x && root->children[i]->action->y == y) {
				root = root->children[i];
			}
		}

		std::cout << sToString(root->state);
	}
}
