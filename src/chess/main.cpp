#include <iostream>
#include <string>
#include <stdexcept>

#include "states.h"
#include "evaluation.h"
#include "../trees.h"

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

		//FEN = "k7/rr6/8/8/8/8/7P/K7 w - - 1 1";
		//FEN = "k7/rr6/8/8/8/8/6rP/K7 w - - 1 1";
		//FEN = "k7/rr6/8/8/8/8/6rP/K6r w - - 1 1";

		//FEN = "k7/3rr3/7r/3K3r/8/8/8/8 w - - 1 1";
		//FEN = "k7/3rr3/7r/3K3r/7r/8/8/8 w - - 1 1";

		//FEN = "k7/8/7r/3K4/8/8/8/8 w - - 1 1";

		FEN = "k7/8/4r3/3K4/4r3/5P2/7P/8 w - - 1 1";
		FEN = "8/8/4r3/1k1K4/4r3/5P2/7P/8 w - - 1 1";

		FEN = "k7/8/8/8/3K4/8/8/8 w - - 1 1";

		FEN = "k7/8/8/8/3K4/8/2b5/8 w - - 1 1";

		FEN = "k7/8/8/8/3K4/8/3b4/8 w - - 1 1";
		FEN = "k7/8/3b4/8/3K4/8/8/8 w - - 1 1";
		FEN = "k7/8/3b4/8/3K4/8/3b4/8 w - - 1 1";

		FEN = "k7/8/8/8/1b1K4/8/8/8 w - - 1 1";
		FEN = "k7/8/8/8/3K1b2/8/8/8 w - - 1 1";
		FEN = "k7/8/8/8/1b1K1b2/8/8/8 w - - 1 1";

		FEN = "k7/8/3b4/8/1b1K1b2/8/3b4/8 w - - 1 1";

		FEN = "k7/8/8/8/3K4/1b6/2b5/8 w - - 1 1";
		FEN = "k7/8/8/8/3K4/1b3b2/2b1b3/8 w - - 1 1";
		FEN = "k7/8/3b4/8/3K4/1b3b2/2bbb3/8 w - - 1 1";
		FEN = "k7/8/3b4/8/3K4/1b6/2bb4/8 w - - 1 1";

		FEN = "k7/8/8/8/3K4/8/2q5/8 w - - 1 1";
		FEN = "k7/8/8/8/3K4/2q5/7P/8 w - - 1 1";

		FEN = "k7/8/8/8/3K4/3r4/7P/8 w - - 1 1";

		FEN = "k7/8/8/8/3K4/2b5/7P/8 w - - 1 1";

		FEN = "k7/8/8/8/q1N1K3/8/7P/8 w - - 1 1";

		// Double en-passant
		FEN = "5B2/6p1/8/4RP1k/4pK1N/6P1/4qP2/8 b - - 1 1";
		FEN = "5B2/8/8/4RPpk/4pK1N/6P1/4qP2/8 w - g6 1 1";
		//FEN = "5B2/8/6P1/4R2k/4pK1N/6P1/4qP2/8 b - - 1 1";

		FEN = "4q1k1/p5b1/6pp/Rpr1Pp2/2p1bB1N/8/P4PPP/3QR1K1 b - - 3 1";
		FEN = "6k1/p5b1/2q3pp/Rpr1Pp2/2p2B1N/8/P4PbP/3QR2K w - - 3 1";
		FEN = "6k1/p5b1/2b3pp/Rpr1Pp2/2p2B1N/8/P4PbP/3QR2K w - - 3 1";
		FEN = "3q2k1/p5b1/7p/Rp1rPp2/2p2p1N/3b1P2/P2Q2PP/2R3K1 b - - 4 1";
		//FEN = "3q4/p3k1b1/6Np/Rp1rPp2/2p2p2/3b1P2/P2Q2PP/2R3K1 w - - 7 1";
		FEN = "3q4/p4kb1/6Np/Rp1rPp2/2p2p2/3b1P2/P2Q2PP/2R3K1 b - - 4 1";
		FEN = "3q1k2/p5b1/6Np/Rp1rPp2/2p2p2/3b1P2/P2Q2PP/2R3K1 b - - 4 1";
		FEN = "4q1k1/5Qb1/7p/Rp1rPp2/4bp1N/2p5/P4PPP/4R2K b - - 1 1";
		FEN = "rnb1kbnr/5ppp/1p1pp3/p7/3Q1B1q/2N2PP1/PPP1P2P/R3KBNR b Kkq - 0 1";

		FEN = "rnbqkbnr/5ppp/1p1pp3/p7/3Q1B2/2N5/PPP1PPPP/R3KBNR w Kkq - 0 1";
		FEN = "rnb1kbnr/5ppp/1p1pp3/p7/3Q1BPq/2N5/PPP1PP1P/R3KBNR w Kkq - 0 1";
		FEN = "rnb1kbnr/5ppp/1p1pp3/p7/3Q1B1q/2N2P2/PPP1P1PP/R3KBNR w Kkq - 0 1";
		//FEN = "rnb1kbnr/5ppp/1p1pp3/p7/3Q1BPq/2N2P2/PPP1P2P/R3KBNR b Kkq g3 0 1";
		FEN = "1nbqkbnr/r1pp1ppp/pp2p3/8/3P1B2/3BPN2/PPP2PPP/RN1QK2R b KQk - 3 1";
		FEN = "1nbqkbnr/r1pp1ppp/pp2p3/8/1q1P1B2/3BPN2/PPP2PPP/RN1QK2R w KQk - 3 1";
		FEN = "1nbqkbnr/r1pp1ppp/pp2p3/8/3P1B2/3BPN2/PqP2PPP/R3K2R w KQk - 3 1";
		//FEN = "1nbqkbnr/r1pp1ppp/pp2p3/8/1q1P1B2/3BPN2/PPP2PPP/R3K2R w KQk - 3 1";
		FEN = "1nbqkbnr/r1pp1ppp/pp2p3/8/3P1B2/3BPN2/PqPK1PPP/R6R b KQk - 3 1";
		FEN = "1nbqkbnr/r1pp1ppp/pp2p3/8/3P1B2/q2BPN2/P1PK1PPP/R6R w KQk - 3 1";

		FEN = "1nbqkbnr/r1pp1ppp/pp2p3/8/3P1B2/3BPN2/PPP2PPP/RN1QK2R b KQk - 3 1";
		FEN = "1nbqkbnr/r1pp1ppp/pp2p3/8/3P1B2/3BPN2/PPP2PPq/RN1QK2R w KQk - 3 1";
		FEN = "k7/8/8/8/8/8/6Pq/4K2R w KQk - 3 1";
		//FEN = "4k3/8/8/8/8/8/7q/R3K2R w KQk - 3 1";
		//FEN = "4k3/8/8/8/8/8/7q/13K21 w KQk - 3 1";
		//FEN = "4k3/8/8/8/8/8/7r/13K21 w KQk - 3 1";
		//FEN = "4k3/8/8/8/8/8/7b/13K21 w KQk - 3 1";
		//FEN = "4k3/8/8/8/8/8/8/R3K2R w KQk - 3 1";
	}
	try {
		s = FEN;
	} catch (const std::invalid_argument& ia) {
		std::cerr << ia.what() << std::endl;
		return 1;
	}

	std::cout << FEN << std::endl;
	std::cout << s.toFEN() << std::endl;
	s.board.print(WHITE, true);

	Node *root = new Node{&s, nullptr, 0, nullptr};

	std::cerr << "Current state:" << std::endl << root->state->toString() << "\n" << std::endl;

	unsigned int depth = 1;
	bool player = true;

	Evaluation e = bestAction(root, depth);

	std::cout << root->amtChildren << " valid move(s)" << std::endl;
	for (unsigned int i=0; i<root->amtChildren; i++) {
		std::cout << root->children[i]->action->toString() << std::endl;
	}
	(void) e;
	(void) player;

	return 0;
}


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

	//FEN = "k7/8/8/8/8/7P/1p6/K7 b - - 1 1";
	FEN = "k7/8/8/8/8/1p5P/8/K7 w - - 1 1";
	FEN = "8/7P/8/8/8/8/K7/3k4 w - - 140 1";
	FEN = "7Q/8/8/8/8/8/8/K3k3 b - - 142 1";
	FEN = "3k4/7R/3K4/8/8/8/8/8 b - - 140 1";

	FEN = "4q1k1/p5b1/1p4pp/nRr1Pp2/Q1p2B2/3b1N2/P4PPP/4R1K1 w - - 2 26";
	// Both err
	FEN = "4q1k1/p5b1/6pp/Rpr1Pp2/2p1bB1N/8/P4PPP/3QR1K1 b - - 3 1";
	FEN = "3q1k2/p5b1/6Np/Rp1rPp2/2p2p2/3b1P2/P2Q2PP/2R3K1 b - - 4 1";

	FEN = STARTING_FEN;
	FEN = "1nb1kbnr/r1pp1ppp/pp2pq2/8/3P1B2/3BPN2/PPP2PPP/RN1QK2R w KQk - 4 1";
	FEN = "1nb1kbnr/r1pp1pp1/pp2p2p/6N1/3P4/3BP3/PPP2PPP/RN1QK2R w KQk - 0 1";
	FEN = "1nb1k1n1/3p1p2/4p1p1/1pP4r/3P4/p3PP2/2P1K2P/1Q5R w - - 0 1";
	FEN = "Q1bk2n1/3p1p2/3R2p1/1pP2n2/8/4PPK1/2r5/8 w - - 3 1";

	FEN = "1nb1kbnr/r1pp1ppp/pp2pq2/6B1/3P4/3BPN2/PPP2PPP/RN1QK2R b KQk - 5 1";
	FEN = STARTING_FEN;
	FEN = "1nb1kbnr/5p2/1prpp1pp/p7/2NP4/2PB1N2/PP3PPP/R2QK2R w KQk - 2 1";
	FEN = "1nb1kbnr/5p2/1prpp2p/p7/2NP2p1/1QPB1N2/PP1K1PPP/R6R w k - 0 1";
	// Played with black as computer
	// Black trades queen for bishop early on, otherwise decent play
	FEN = "3k2nr/3b1p2/3p3b/p1rNn2p/5QpN/1BP5/PP1K1PPP/R6R w - - 11 1";
	FEN = STARTING_FEN;
	FEN = "1nbqkbnr/r1pp1ppp/pp2p3/8/3P1B2/3BPN2/PPP2PPP/RN1QK2R b KQk - 3 1";
	FEN = STARTING_FEN;
	FEN = "5k1r/1bQRbp2/1pN5/7p/1P6/4P1P1/5PP1/6K1 w - - 1 1";
	FEN = STARTING_FEN;
	FEN = "rnbqkb1r/1p2pppp/p2p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6";

	try {
		sp = new Gamestate {FEN};
	} catch (const std::invalid_argument& ia) {
		std::cerr << ia.what() << std::endl;
		return 1;
	}

	Node *root = new Node{sp, nullptr, 0, nullptr};

	unsigned int depth = 5;
	bool player = false;
	bool playerWhite = true;

	Evaluation e;

	while (!gameOver(root->state)) {
		Node* nextRoot;
		Action a{{0,0},{0,0}};
		std::string tmp;

		// Print state
		std::cout << root->state->toString() << std::endl;
		root->state->board.print(playerWhite ? WHITE : BLACK, true);
		std::cout << "State evaluation:\t" << evaluation(root->state) << std::endl;

		if (player && root->state->whiteToMove == playerWhite) {
			// Generate child-nodes
			genChildren(root);
			// No input-validation
			std::cin >> tmp;
			Coordinate from = {tmp};
			std::cin >> tmp;
			Coordinate to = {tmp};
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
			std::cout << "Node evaluation:\t" << (root->state->whiteToMove ? 1 : -1) * e.evaluation << std::endl;
			std::cout << "Computer plays:\t" << e.action->toString() << std::endl;
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
		std::cout << "\n";
	}

	// Print terminal state
	std::cout << root->state->toString() << std::endl;
	root->state->board.print(playerWhite ? WHITE : BLACK, true);

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

