#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <string>
#include <vector>

struct Action;
struct Gamestate;

unsigned int getActions(Gamestate const* statep, std::vector<Gamestate*>& gamestates, std::vector<Action*>& actions);

float evaluation(Gamestate const* statep);

#endif
