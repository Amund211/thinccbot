#include <iostream>
#include <string>

#include "states.h"
#include "../trees.h"
#include "../game.h"

bool gameOver(Gamestate const*);
float evaluation(Gamestate const*);

int main() {
	Node *root = new Node;
	root->state = new Gamestate();

	unsigned int depth = 9;
	bool player = true;
	unsigned int ply = 0;

	Evaluation e = bestAction(root, depth);
	std::cout <<
		"Best action is:\t" << aToString(e.action) << std::endl <<
		"Evaluation:\t" << e.evaluation << std::endl;

	std::cout << sToString(root->state);
	while (!gameOver(root->state)) {
		Node* nextRoot;
		unsigned int column;


		std::cout << "Current state evaluation:\t" << evaluation(root->state) << std::endl;
		if (player && root->state->yellowToMove) {
			// No input-validation
			std::cin >> column;
		} else {
			e = bestAction(root, depth);
			column = e.action->column;
			std::cout << "Node evaluation:\t" << e.evaluation << std::endl;
		}

		// Find the corresponding child-node
		for (unsigned int i=0; i<root->amtChildren; i++) {
			if (root->children[i]->action->column == column) {
				nextRoot = root->children[i];
			} else {
				freeSubtree(root->children[i]);
			}
		}

		// Free old root
		delete root;

		root = nextRoot;

		std::cout << "\n" << sToString(root->state);
		std::cout << "Ply: " << ++ply << std::endl;
	}
	std::cout << e.evaluation << std::endl;
	std::cout << evaluation(root->state) << std::endl;
}
