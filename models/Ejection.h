//
// Created by dragoon on 3/27/18.
//

#ifndef ALMOSTAGARIO_EJECTION_H
#define ALMOSTAGARIO_EJECTION_H


#include "GameObject.h"

class Ejection : public GameObject {

public:
    int playerId;

    Ejection(int id, double x, double y, int playerId, int updateTick);

};


#endif //ALMOSTAGARIO_EJECTION_H
