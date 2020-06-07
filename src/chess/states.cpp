#include <regex>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cmath>

#include "../game.h"

#include "utils.h"
#include "states.h"
#include "pieces.h"

const std::regex FEN_regex(
	// Ranks index 1-8
	"([1-8kqrbnpKQRBNP]{1,8})\\/"
	"([1-8kqrbnpKQRBNP]{1,8})\\/"
	"([1-8kqrbnpKQRBNP]{1,8})\\/"
	"([1-8kqrbnpKQRBNP]{1,8})\\/"
	"([1-8kqrbnpKQRBNP]{1,8})\\/"
	"([1-8kqrbnpKQRBNP]{1,8})\\/"
	"([1-8kqrbnpKQRBNP]{1,8})\\/"
	"([1-8kqrbnpKQRBNP]{1,8}) "
	// To move index 9
	"([wb]) "
	// Castling avaliability index 10
	"(-|(?:K?Q?k?q?)) "
	// En passant target square index 11
	"(-|[a-h][36]) "
	// Halfmove clock index 12
	"(\\d+) "
	// Fullmove clock, not needed
	"(?:\\d+)"
);

void deleteState(Gamestate* state) {
	delete state;
}

void deleteAction(Action* action) {
	delete action;
}

Delta Coordinate::step() const
{
	// Return Delta scaled by 1/Linf
	// Make sure isDiagonal || isStraight
	int Linf = infNorm();
	return {rank / Linf, file / Linf};
}

std::string Coordinate::toString() const {
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

Action::Action(Coordinate from, Coordinate to, Piece promotionPiece)
	: from{from}, to{to}, promotionPiece{promotionPiece} {}

bool Action::operator ==(const Action& b) const
{
	return from == b.from &&
		to == b.to &&
		promotionPiece == b.promotionPiece;
}

//const std::regex FEN_regex("(\\S+) ([wb]) ([KQkq-]{1,4}) (\\S{1,2}) (\\d+) (\\d+)");


#if 0
	// Read board
	target = FEN_match[1];
	for (unsigned int i=0; i<target.size(); i++)
	{
		c = target[i];
		if (c == '/')
		{
			rank--; // Rank-separator
			file = 0;
			continue;
		}
		if (isdigit(c))
		{
			// Add empty squares
			for (unsigned int j = 0; j<c-'0'; j++)
			{
				board.set({rank, file}, NONE);
				file++;
			}
		}
		else
		{
			// Add piece
			board.set({rank, file}, symbolToPiece(c));
			file++;
		}
	}
#endif

Gamestate::Gamestate(std::string FEN)
	: whiteCastle{false, false}, blackCastle{false, false}
{
	// Set state according to provided FEN
	// Input is barely validated

	std::string target;
	std::smatch FEN_match;
	char c;
	Piece tmpPiece;
	Castle *castleTarget;

	std::regex_match(FEN, FEN_match, FEN_regex);

	if (FEN_match.empty())
		throw std::invalid_argument("Improper FEN");

	// Read board
	for (int rank=7; rank>=0; rank--) {
		int file = 0;
		target = FEN_match[8 - rank];
		for (unsigned int i=0; i<target.size(); i++) {
			c = target[i];
			if (isdigit(c)) {
				// Add empty squares
				for (int j = 0; j<c-'0'; j++) {
					board.set({rank, file}, NONE);
					file++;
				}
			} else {
				// Add piece
				board.set({rank, file}, symbolToPiece(c));
				file++;
			}
		}
	}

	// Active color
	whiteToMove = FEN_match[9] == 'w';

	// Castling avaliability one or more of 'K', 'Q', 'k', 'q'
	target = FEN_match[10];

	if (target[0] != '-') {
		for (unsigned int i=0; i<target.size(); i++) {
			c = target[i];
			tmpPiece = symbolToPiece(c);

			// Obtain reference to whites/blacks castle struct
			if (pieceColor(tmpPiece) == WHITE)
				castleTarget = &whiteCastle;
			else
				castleTarget = &blackCastle;

			// Set the relevant field
			if (pieceType(tmpPiece) == KING)
				castleTarget->kingside = true;
			else
				castleTarget->queenside = true;
		}
	}
	
	// En passant target square
	if (FEN_match[11] == '-') {
		// No passant pawn
		passantSquare = {-1, -1};
	} else {
		passantSquare = SAN2Coord(FEN_match[11]);
	}
	

	// Halfmove clock (since pawn move or capture)
	rule50Ply = std::stoi(FEN_match[12]);
	
	// Fullmove number is not relevant to the evaluation of the position, and is skipped
}

Gamestate::Gamestate() : Gamestate(STARTING_FEN) {}

std::string Gamestate::toFEN() const
{
	std::ostringstream oss;

	// Board
	unsigned int amtEmpty=0;
	for (int rank=7; rank>=0; rank--) {
		for (int file=0; file<=7; file++) {
			Piece p = board.get({rank, file});
			if (pieceType(p) == NONE) {
				amtEmpty++;
			} else {
				if (amtEmpty) {
					oss << amtEmpty;
					amtEmpty = 0;
				}
				oss << pieceToSymbol[p];
			}
		}
		if (amtEmpty) {
			oss << amtEmpty;
			amtEmpty = 0;
		}
		if (rank != 0)
			oss << "/";
	}

	// To move
	oss << (whiteToMove ? " w " : " b ");

	if (whiteCastle.canCastle() || blackCastle.canCastle()) {
		if (whiteCastle.kingside)
			oss << "K";
		if (whiteCastle.queenside)
			oss << "Q";
		if (blackCastle.kingside)
			oss << "k";
		if (blackCastle.queenside)
			oss << "q";
	} else {
		oss << "-";
	}

	// En passant target square
	if (!passantSquare.isValid())
		oss << " - ";
	else
		oss << " " << passantSquare.toString() << " ";

	// Halfmove clock since last capture/pawn move
	oss << static_cast<unsigned int>(rule50Ply) << " ";

	// Fullmove clock, not stored in this state
	oss << 1;

	return oss.str();
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
	drawBoard(b, WHITE, true);
	throw std::invalid_argument("Board does not have a king of the given color");
}

constexpr float materialValue[7] = {0, 1, 3, 3, 5, 9, 0};

float materialCount(const Board& b)
{
	float count = 0;
	for (int rank=0; rank<8; rank++)
		for (int file=0; file<8; file++) {
			Piece tmpPiece = b.get({rank, file});
			count += pawnDirection(pieceColor(tmpPiece)) * materialValue[pieceType(tmpPiece)];
		}
	return count;
}

std::string aToString(Action const* actionp) {
	std::string s {actionp->from.toString()};
	s += " ";
	s += actionp->to.toString();
	if (pieceType(actionp->promotionPiece) != NONE) {
		s += " ";
		s += pieceToSymbol[actionp->promotionPiece];
	}
	return s;
}

std::string sToString(Gamestate const* statep) {
	return statep->toFEN();
}
