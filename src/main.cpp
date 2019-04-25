#include <iostream>
#include <string>

#include "list.h"
#include "states.h"
#include "trees.h"
#include "game.h"


int main() {
	Node *root = new Node;
	root->statep = new Gamestate;
	*root->statep = { true, 0, 0, 0, 0 };
	root->amtChildren = 2;
	root->children = new Node*[2];

	unsigned int depth = 15;

	genTree(root, depth);

	std::cout << negamax(root, depth) << std::endl;
	std::cout << pprint(bestAction(root, depth)) << std::endl;
}
