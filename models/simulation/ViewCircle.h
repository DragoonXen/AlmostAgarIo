//
// Created by dragoon on 4/11/18.
//

#ifndef ALMOSTAGARIO_VIEWCIRCLE_H
#define ALMOSTAGARIO_VIEWCIRCLE_H


#include "../PlayerPart.h"
#include "../Food.h"
#include "../../Utils.h"

class ViewCircle {
public:
    double cX, cY;
    double radius;

    ViewCircle(const PlayerPart &playerPart, int count);

    bool canSee(const Food &food) const;

    bool canSee(const PlayerPart &part) const;

    bool canSee(int i, int j) const {
        double cX = i * Constants::COORDS_IN_ONE_FOOD_FIELD + Constants::HALF_COORDS_IN_ONE_FOOD_FIELD;
        double cY = j * Constants::COORDS_IN_ONE_FOOD_FIELD + Constants::HALF_COORDS_IN_ONE_FOOD_FIELD;
        return Utils::dist(cX, cY, this->cX, this->cY) + Constants::ENOUGHT_SEEN_DISTANCE <= this->radius;
    }

};


#endif //ALMOSTAGARIO_VIEWCIRCLE_H
