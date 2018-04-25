//
// Created by dragoon on 3/27/18.
//

#ifndef ALMOSTAGARIO_VIRUS_H
#define ALMOSTAGARIO_VIRUS_H

#include <string>
#include "GameObject.h"
#include "Ejection.h"
#include "PlayerPart.h"

class Virus : public GameObject {
public:
    double split_angle;

    Virus(int id, double x, double y, double m, int updateTick);

    void eat(const Ejection &eject);

    double canHurt(const PlayerPart &object) const;
};


#endif //ALMOSTAGARIO_VIRUS_H
