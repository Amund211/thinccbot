#include "board.h"

#include "pieces.h"

#include <string>
#include <array>
#include <cassert>

// Construct a coordinate from its representation in standard algebraic notation
// ex: e2, g6
Coordinate::Coordinate(std::string SAN)
	: rank{SAN[1] - '1'}, file{SAN[0] - 'a'}
{
	if (!isValid())
		throw std::invalid_argument(SAN + std::string(" is an invalid SAN coordinate"));
}

Delta Coordinate::step() const
{
	// Return Delta scaled by 1/Linf
	assert(isDiagonal() || isStraight());
	int Linf = infNorm();
	return {rank / Linf, file / Linf};
}

std::string Coordinate::toSAN() const {
	std::string o;
	o += 'a' + file;
	o += std::to_string(rank + 1);
	return o;
}

Coordinate Coordinate::operator +(const Coordinate& b) const
{
	return {rank + b.rank, file + b.file};
}

Coordinate& Coordinate::operator +=(const Coordinate& b)
{
	rank += b.rank;
	file += b.file;
	return *this;
}

Coordinate Coordinate::operator -(const Coordinate& b) const
{
	return {rank - b.rank, file - b.file};
}

Coordinate& Coordinate::operator -=(const Coordinate& b)
{
	rank -= b.rank;
	file -= b.file;
	return *this;
}

Coordinate Coordinate::operator -() const
{
	return {-rank, -file};
}

bool Coordinate::operator ==(const Coordinate& b) const
{
	return rank == b.rank && file == b.file;
}

bool Coordinate::operator !=(const Coordinate& b) const
{
	return rank != b.rank || file != b.file;
}

Piece Board::get(Coordinate pos) const
{
	uint8_t elem = _board[(7 - pos.rank) * 4 + (pos.file >> 1)];
	if (pos.file % 2 == 0)
	{
		// Big nibble
		return (elem >> 4);
	}
	else
	{
		// Small nibble
		return elem & 15;
	};
}

void Board::set(Coordinate pos, Piece piece)
{
	unsigned int index = (7 - pos.rank) * 4 + (pos.file >> 1);
	uint8_t elem = _board[index];
	if (pos.file % 2 == 0)
	{
		// Big nibble
		_board[index] = (elem & ~(15<<4)) | (piece << 4);
	}
	else
	{
		// Small nibble
		_board[index] = (elem & ~15) | piece;
	};
}

void Board::move(Coordinate from, Coordinate to)
{
	set(to, get(from));
	set(from, NONE);
}

void Board::print(Color perspective, bool colorTerminal, std::ostream& stream)
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

	for (int rank=topRank; rank>=0 && rank<8; rank+=rankDirection) {
		stream << rank + 1 << " ";
		for (int file=leftFile; file>=0 && file<8; file-=rankDirection) {
			if (colorTerminal) {
				if ((file ^ rank) & 1)
					// White square - colored red
					stream << "\033[1;37;101m";
				else
					// Black square - colored blue
					stream << "\033[1;37;104m";
			}
			stream << " " << pieceToUnicode[get({rank, file})] << " ";
		}

		if (colorTerminal)
			stream << "\033[0m";

		stream << std::endl;
	}

	stream << "  ";
	for (int file=leftFile; file>=0 && file<8; file-=rankDirection)
		stream << " " << static_cast<char>('a' + file) << " ";
	stream << std::endl;
}

Coordinate findKing(const Board& b, Color c)
{
	for (int rank=0; rank<8; rank++) {
		for (int file=0; file<8; file++) {
			Piece p = b.get({rank, file});
			if (pieceType(p) == KING && pieceColor(p) == c)
				return {rank, file};
		}
	}

	//b.print(WHITE, true);
	throw std::invalid_argument("Board does not have a king of the given color");
}
