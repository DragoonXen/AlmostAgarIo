//
// Created by dragoon on 3/27/18.
//

#ifndef ALMOSTAGARIO_CONTEXT_H
#define ALMOSTAGARIO_CONTEXT_H


#include <list>
#include "models/PlayerPart.h"
#include "models/Food.h"
#include "models/Virus.h"
#include "models/Ejection.h"
#include "models/Player.h"

class Context {
public:
    int tickIndex;
    double prevDestX, prevDestY;

    bool dummy;
    std::array<Player, 4> players;
    Player * me;
    std::list<Food> food;
    std::list<Virus> viruses;
    std::list<Ejection> ejections;
    std::list<PlayerPart *> allPlayersParts;

    void updatePlayerParts();

    Context(int tickIndex);

    Context(const Context &obj) {
        this->tickIndex = obj.tickIndex;
        this->prevDestX = obj.prevDestX;
        this->prevDestY = obj.prevDestY;
        this->dummy = obj.dummy;
        this->players = obj.players;
        this->me = this->players.data() + obj.me->id;
        this->food = obj.food;
        this->viruses = obj.viruses;
        this->ejections = obj.ejections;
    }

    int getTickIndex() const;

    double getPrevDestX() const;

    double getPrevDestY() const;

    void nextTick(double currDestX, double currDestY);
};


#endif //ALMOSTAGARIO_CONTEXT_H
