//
// Created by dragoon on 4/12/18.
//

#include "EnemyDirectionMem.h"
#include "../../Utils.h"

EnemyDirectionMem::EnemyDirectionMem(int fragId, double x, double y, double dx, double dy, double cX, double cY, double mass, bool exact)
        : fragId(fragId), x(x), y(y), cX(cX), cY(cY), mass(mass), exact(exact) {
    this->dx = dx;
    this->dy = dy;
    double len = Utils::len(dx, dy);
    double maxSpeed = Constants::SPEED_FACTOR() / sqrt(mass);
    if (len > maxSpeed) {
        double mul = maxSpeed / len;
        this->dx *= mul;
        this->dy *= mul;
    }
}
