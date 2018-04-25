//
// Created by dragoon on 4/11/18.
//

#include "ViewCircle.h"
#include "../../Constants.h"
#include "../../Utils.h"

ViewCircle::ViewCircle(const PlayerPart &playerPart, int count) {
    if (playerPart.getDx() == 0 && playerPart.getDy() == 0) {
        this->cX = playerPart.getX();
        this->cY = playerPart.getY();
    } else {
        double speed = Utils::len(playerPart.getDx(), playerPart.getDy());
        this->cX = playerPart.getX() + playerPart.getDx() / speed * Constants::VIS_SHIFT;
        this->cY = playerPart.getY() + playerPart.getDy() / speed * Constants::VIS_SHIFT;
    }
    if (count == 1) {
        this->radius = playerPart.getRadius() * Constants::VIS_FACTOR;
    } else {
        this->radius = playerPart.getRadius() * Constants::VIS_FACTOR_FR * sqrt(count);
    }
}

bool ViewCircle::canSee(const Food &food) const {
    return Utils::dist(food.getX(), food.getY(), cX, cY) - food.getRadius() < this->radius;
}

bool ViewCircle::canSee(const PlayerPart &part) const {
    return Utils::dist(part.getX(), part.getY(), cX, cY) - part.getRadius() < this->radius;
}
