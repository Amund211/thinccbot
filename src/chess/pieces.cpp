#include "pieces.h"

Piece symbolToPiece(char symbol)
{
	for (Piece p=0; p<=15; p++)
		if (pieceToSymbol[p] == symbol)
			return p;
	return UNKNOWN;
}

