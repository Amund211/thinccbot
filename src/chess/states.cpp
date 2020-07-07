#include <regex>
#include <string>
#include <sstream>
#include <stdexcept>

#include "../game.h"

#include "states.h"
#include "pieces.h"

extern const std::string STARTING_FEN {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};

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

Action::Action(Coordinate from, Coordinate to, Piece promotionPiece)
	: from{from}, to{to}, promotionPiece{promotionPiece} {}

std::string Action::toAN() const {
	std::string s {from.toString()};
	s += to.toString();
	if (pieceType(promotionPiece) != NONE) {
		s += pieceToSymbol[promotionPiece];
	}
	return s;
}

std::string Action::toString() const {
	std::string s {from.toString()};
	s += " ";
	s += to.toString();
	if (pieceType(promotionPiece) != NONE) {
		s += "=";
		s += pieceToSymbol[promotionPiece];
	}
	return s;
}

bool Action::operator ==(const Action& b) const
{
	return from == b.from &&
		to == b.to &&
		promotionPiece == b.promotionPiece;
}

Gamestate::Gamestate(std::string FEN)
	: whiteCastle{false, false}, blackCastle{false, false}
{
	// Set state according to provided FEN
	// The input is trusted; only basic validation is done

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
			if (file >= 8)
				throw std::invalid_argument("Too many pieces on a single rank");

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
		// Construct the coordinate from the SAN-representation
		passantSquare = {FEN_match[11]};
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
