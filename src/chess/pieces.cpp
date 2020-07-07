#include "pieces.h"

Piece symbolToPiece(char symbol)
{
	// Could've made a switch, but this is rarely used
	for (Piece p=0; p<=15; p++)
		if (pieceToSymbol[p] == symbol)
			return p;
	return UNKNOWN;
}

