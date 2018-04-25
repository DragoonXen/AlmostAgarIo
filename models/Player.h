//
// Created by dragoon on 3/30/18.
//

#ifndef ALMOSTAGARIO_PLAYER_H
#define ALMOSTAGARIO_PLAYER_H

#include "PlayerPart.h"
#include "simulation/EnemyDirectionMem.h"
#include <list>

class Player {
public:
    int id;

    Player();

    Player(int id);

    double aimX, aimY;

    std::list<PlayerPart> parts;

};


#endif //ALMOSTAGARIO_PLAYER_H
