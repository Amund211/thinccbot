// Users must include <string> "list.h" "states.h"

#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

unsigned int getActions(const Gamestate &statep, List<Gamestate> &gamestates, List<Action> &actions);

float evaluation(const Gamestate &state);

std::string pprint(Action action);
#endif
