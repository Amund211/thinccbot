#include <iostream>
#include <string>
#include <stdexcept>

#include "utils.h"
#include "states.h"
#include "../trees.h"
#include "../game.h"

constexpr bool TEST = false;

int test()
{
	std::string FEN;
	Gamestate s;
	if (false)
	{
		FEN = STARTING_FEN;
	} else {
		FEN = "4q1k1/p5b1/1p4pp/nRr1Pp2/Q1p2B2/3b1N2/P4PPP/4R1K1 w - - 2 26";

		FEN = "8/8/1p6/p1pppppp/PP6/2PPPPPP/8/8 b - - 1 1";
		FEN = "k7/8/8/8/8/1p5P/8/K7 w - - 1 1";
		FEN = "k7/8/8/8/8/8/1p5P/K7 w - - 1 1";
		FEN = "k7/8/8/8/8/8/1p5P/1K6 b - - 1 1";

		FEN = "k7/8/8/8/8/1p5P/8/K7 w - - 1 1";

		FEN = "k7/8/8/8/8/2n5/8/K7 w - - 1 1";
		FEN = "k7/8/8/8/3K4/2n5/8/8 w - - 1 1";

		FEN = "k7/8/8/4n3/3K4/2n5/8/8 w - - 1 1";

		FEN = "k7/8/6n1/4n3/3K4/2n5/n7/8 w - - 1 1";
		//FEN = "k7/8/6n1/8/3K4/8/n7/8 w - - 1 1";
	}
	try {
		s = FEN;
	} catch (const std::invalid_argument& ia) {
		std::cerr << ia.what() << std::endl;
		return 1;
	}

	std::cout << FEN << std::endl;
	std::cout << s.toFEN() << std::endl;
	drawBoard(s.board, WHITE, true);

	Node *root = new Node{&s, nullptr, 0, nullptr};

	std::cerr << "Current state:" << std::endl << sToString(root->state) << "\n" << std::endl;

	// The last move you should look at should be your opponents move
	// Use even depths
	unsigned int depth = 1;
	bool player = true;

	Evaluation e = bestAction(root, depth);

	std::cout << root->amtChildren << " valid moves" << std::endl;
	for (unsigned int i=0; i<root->amtChildren; i++) {
		std::cout << aToString(root->children[i]->action) << std::endl;
	}
	(void) e;
	(void) player;

	return 0;
}


bool gameOver(Gamestate const*);

int play()
{
	std::string FEN;
	Gamestate* sp;
	FEN = "4q1k1/p5b1/1p4pp/nRr1Pp2/Q1p2B2/3b1N2/P4PPP/4R1K1 w - - 2 26";
	FEN = "8/pppppppp/8/8/8/8/PPPPPPPP/8 w - - 1 1";

	FEN = "8/p6P/8/8/8/8/8/8 w - - 1 1";
	FEN = "8/8/8/8/8/8/p6P/8 b - - 1 1";
	FEN = "8/ppp5/8/PPP5/8/8/8/8 w - - 1 1";

	FEN = "8/p6p/7P/6N1/8/8/8/8 w - - 1 1";
	FEN = "8/pppppppp/8/8/8/8/PPPPPPPP/8 w - - 1 1";

	FEN = "8/8/1p6/p1pppppp/PP6/2PPPPPP/8/8 b - - 1 1";
	FEN = "8/8/8/p1PPP3/P1P4p/7P/8/5n2 b - - 0 1";

	FEN = "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 1 1";

	FEN = "k7/8/8/8/8/1p5P/8/K7 w - - 1 1";

	//FEN = "k7/8/8/8/8/7P/1p6/K7 b - - 1 1";
	try {
		sp = new Gamestate {FEN};
	} catch (const std::invalid_argument& ia) {
		std::cerr << ia.what() << std::endl;
		return 1;
	}

	Node *root = new Node{sp, nullptr, 0, nullptr};

	std::cerr << "Current state:" << std::endl << sToString(root->state) << "\n" << std::endl;

	unsigned int depth = 10;
	bool player = false;
	bool playerWhite = true;

	Evaluation e = bestAction(root, depth);
	std::cout <<
		"Best action is:\t" << aToString(e.action) << std::endl <<
		"Evaluation:\t" << e.evaluation << std::endl;

	std::cout << sToString(root->state) << std::endl;
	drawBoard(root->state->board, playerWhite ? WHITE : BLACK, true);
	while (!gameOver(root->state)) {
		Node* nextRoot;
		Action a{{0,0},{0,0}};
		std::string tmp;

		std::cout << "Current state evaluation:\t" << evaluation(root->state) << std::endl;
		if (player && root->state->whiteToMove == playerWhite) {
			// No input-validation
			std::cin >> tmp;
			Coordinate from = SAN2Coord(tmp);
			std::cin >> tmp;
			Coordinate to = SAN2Coord(tmp);
			std::cin >> tmp;

			if (tmp == "p") {
				a = {from, to};
			} else {
				Piece pp = symbolToPiece(tmp[0]);
				a = {from, to, pp};
			}
		} else {
			e = bestAction(root, depth);
			a = *e.action;
			std::cout << "Node evaluation:\t" << e.evaluation << std::endl;
		}

		// Find the corresponding child-node
		for (unsigned int i=0; i<root->amtChildren; i++) {
			if (*root->children[i]->action == a) {
				nextRoot = root->children[i];
			} else {
				freeSubtree(root->children[i]);
			}
		}

		// Free old root
		delete root;

		root = nextRoot;

		std::cout << "\n" << sToString(root->state) << std::endl;
		drawBoard(root->state->board, playerWhite ? WHITE : BLACK, true);
	}
	std::cout << e.evaluation << std::endl;
	std::cout << evaluation(root->state) << std::endl;

	return 0;
}

int main()
{
	if (TEST)
		return test();
	else
		return play();
}

