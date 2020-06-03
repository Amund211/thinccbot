#include "utils.h"

#include <iostream>
#include <string>

extern const std::string STARTING_FEN {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};

Coordinate SAN2Coord(std::string SAN)
{
	return {SAN[1] - '1', SAN[0] - 'a'};
}

void drawBoard(const Board& b, Color perspective, bool colorTerminal)
{
	int topRank, leftFile, rankDirection;
	if (perspective == WHITE) {
		topRank = 7;
		leftFile = 0;
		rankDirection = -1;
	} else {
		topRank = 0;
		leftFile = 7;
		rankDirection = 1;
	}

	std::cout << "  ";
	for (int file=leftFile; file>=0 && file<8; file-=rankDirection)
		std::cout << " " << static_cast<char>('a' + file) << " ";
	std::cout << std::endl;

	for (int rank=topRank; rank>=0 && rank<8; rank+=rankDirection) {
		std::cout << rank + 1 << " ";
		for (int file=leftFile; file>=0 && file<8; file-=rankDirection) {
			if (colorTerminal) {
				if ((file ^ rank) & 1) 
					// White square - colored red
					std::cout << "\033[1;37;101m";
				else
					// Black square - colored blue
					std::cout << "\033[1;37;104m";
			}
			std::cout << " " << pieceToUnicode[b.get({rank, file})] << " ";
		}

		if (colorTerminal)
			std::cout << "\033[0m";

		std::cout << std::endl;
	}
}

