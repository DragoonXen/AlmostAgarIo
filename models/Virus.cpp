//
// Created by dragoon on 3/27/18.
//

#include "Virus.h"
#include "../Constants.h"
#include "PlayerPart.h"

Virus::Virus(int id, double x, double y, double m, int updateTick) : GameObject(id, x, y, 0.f, 0.f, m, Constants::VIRUS_RADIUS(), updateTick) {}


void Virus::eat(const Ejection &eject) {
    mass += eject.getMass();
    split_angle = atan2(eject.getDy(), eject.getDx());
}


double Virus::canHurt(const PlayerPart &object) const {
    if (object.getRadius() < radius) {
        return INFINITY;
    }
    double qdist = sqrDist(object);
    double tR = radius * Constants::VIRUS_HURT_RAD + object.getRadius();
    if (qdist < tR * tR) {
        return qdist;
    }
    return INFINITY;
}