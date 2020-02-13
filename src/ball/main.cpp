#include <iostream>
#include <string>

#include "../list.h"
#include "../states.h"
#include "../trees.h"
#include "../game.h"


int main() {
	Node *root = new Node;
	root->state = { true, 0, 0, 0, 0 };
	root->amtChildren = 2;
	root->children = new Node*[2];

	unsigned int depth = 15;

	Evaluation e = bestAction(root, depth);
	std::cout <<
		"Current player:\t" << (root->state.whitetoMove ? "white" : "black") << std::endl <<
		"Best action is:\t" << aToString(e.action) << std::endl <<
		"Evaluation:\t" << e.evaluation << std::endl;
}
