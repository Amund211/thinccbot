#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

#include "states.h"
#include "../game.h"

bool Column::isFull() const
{
	return stack[5] != 0;
}

void Column::drop(int8_t color)
{
	for (unsigned int i=0; i<6; i++) {
		if (!stack[i]) {
			stack[i] = color;
			break;
		}
	}
}

Gamestate::Gamestate(bool yellowToMove)
	: yellowToMove{yellowToMove} {}

void deleteState(Gamestate* state) {
	delete state;
}

void deleteAction(Action* action) {
	delete action;
}

int scoreLine(Gamestate const* statep, int x0, int y0, int xinc, int yinc)
{
	int score = 0;
	for (int inc=0; inc<4; inc++)
		score += statep->columns[x0 + inc*xinc].stack[y0 + inc*yinc];

	if (score == 4)
		return 4;
	else if (score == -4)
		return -4;
	else
		return 0;
}

int8_t won(Gamestate const* statep)
{
	int score = 0;
	for (unsigned int y=0; y<6; y++) {
		for (unsigned int x=0; x<7; x++) {
			if (x<=3) {
				// Across
				score += scoreLine(statep, x, y, 1, 0);
				if (y >= 3) {
					// Diagonal down
					score += scoreLine(statep, x, y, 1, -1);
				} else {
					// Diagonal up
					score += scoreLine(statep, x, y, 1, 1);
					// Up
					score += scoreLine(statep, x, y, 0, 1);
				}
			} else if (y<3) {
				// Up
				score += scoreLine(statep, x, y, 0, 1);
			}
		}
	}

	if (score > 0)
		return 1;
	else if (score < 0)
		return -1;
	else
		return 0;
}

bool gameOver(Gamestate const* statep)
{
	if (won(statep))
		return true;

	for (unsigned int i=0; i<7; i++)
		if (!statep->columns[i].isFull())
			return false;

	return true;
}

void genChildren(Gamestate const* statep, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions) {
	if (won(statep)) {
		return;
	}

	for (unsigned int i=0; i<7; i++) {
		if (!statep->columns[i].isFull()) {
			actions.push_back(new Action{i});
			Gamestate* newstatep = new Gamestate{*statep};
			gamestates.push_back(newstatep);
			newstatep->columns[i].drop(statep->yellowToMove ? 1 : -1);
			newstatep->yellowToMove = !newstatep->yellowToMove;
		}
	}
	return;

	for (unsigned int i=1; i<actions.size(); i++) {
		// Prioritize moves in the center
		switch (actions[i]->column) {
			case 2:
				std::swap(gamestates[0], gamestates[i]);
				std::swap(actions[0], actions[i]);
				break;
			case 3:
				std::swap(gamestates[0], gamestates[i]);
				std::swap(actions[0], actions[i]);
				break;
			case 4:
				std::swap(gamestates[1], gamestates[i]);
				std::swap(actions[1], actions[i]);
				break;
		}
	}

	return;
}

float evaluateLine(Gamestate const* statep, int8_t color, unsigned int x0, unsigned int y0, int xinc, int yinc, unsigned int amtinc)
{
	// The running score
	float score = 0;

	// Consecutive points on the line that are the given color or blank
	unsigned int potential = 0;

	// Correctly filled points in the current segment
	unsigned int filled = 0;

	for (unsigned int i=0; i<amtinc; i++) {
		switch (color * statep->columns[x0 + xinc*i].stack[y0 + yinc*i]) {
			case (1):
				filled++;
				// Fallthrough
			case 0:
				potential++;
				break;
			default:
				// Streak broken
				if (potential >= 4) {
					// Some metric
					score += potential*filled;
				}
				potential = 0;
				filled = 0;
		}
	}

	// Hit the wall
	if (potential >= 4) {
		// Some metric
		score += potential*filled;
	}

	return score / 100;
}

float evaluatePlayer(Gamestate const* statep, int8_t color)
{
	float score = 0;

	for (unsigned int x=0; x<7; x++)
		score += evaluateLine(statep, color, x, 0, 0, 1, 6);

	for (unsigned int y=0; y<6; y++)
		score += evaluateLine(statep, color, 0, y, 1, 0, 7);

	// Upwards diagonals
	score += evaluateLine(statep, color, 0, 2, 1, 1, 4);
	score += evaluateLine(statep, color, 0, 1, 1, 1, 5);
	score += evaluateLine(statep, color, 0, 0, 1, 1, 6);
	score += evaluateLine(statep, color, 1, 0, 1, 1, 6);
	score += evaluateLine(statep, color, 2, 0, 1, 1, 5);
	score += evaluateLine(statep, color, 3, 0, 1, 1, 4);

	// Downwards diagonals
	score += evaluateLine(statep, color, 0, 3, 1, -1, 4);
	score += evaluateLine(statep, color, 0, 4, 1, -1, 5);
	score += evaluateLine(statep, color, 0, 5, 1, -1, 6);
	score += evaluateLine(statep, color, 1, 5, 1, -1, 6);
	score += evaluateLine(statep, color, 2, 5, 1, -1, 5);
	score += evaluateLine(statep, color, 3, 5, 1, -1, 4);

	return score;
}

float evaluation(Gamestate const* statep) {
	int8_t winner = won(statep);
	switch (winner) {
		case 1:
			return 100;
		case -1:
			return -100;
	}

	float score = evaluatePlayer(statep, 1) - evaluatePlayer(statep, -1);

	// Prioritize winning over getting a good evaluation
	if (score >= 100)
		return 99.9;
	else if (score <= -100)
		return -99.9;
	else
		return score;
}

std::string Action::toString() {
	return std::to_string(column);
}

std::string Gamestate::toString() {
	std::string s{"To move: "};
	s.append(yellowToMove ? "yellow\n" : "red\n");
	s.append(" 0 1 2 3 4 5 6 \n");
	for (int y=5; y>=0; y--) {
		s.append("|");
		for (unsigned int x=0; x<7; x++) {
			if (columns[x].stack[y] == 0) {
				s.append(" |");
			} else {
				s.append(columns[x].stack[y] == 1 ? "Y|" : "R|");
			}
		}
		s.append("\n");
	}
	return s;
}
