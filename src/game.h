// Users must include <string> "list.h" "states.h"

#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

unsigned int getActions(Gamestate *statep, List<Gamestate*> *gamestateps, List<Action> *actions);

float evaluation(Gamestate *statep);

std::string pprint(Action action);
#endif
