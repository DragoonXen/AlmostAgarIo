//
// Created by dragoon on 3/27/18.
//

#include "PlayerPart.h"
#include "../SimpleLog.h"
#include <math.h>

PlayerPart::PlayerPart(int id, int playerId, double x, double y, double sx, double sy, double m, double r, int updateTick, int ttf) : GameObject(id,
                                                                                                                                                 x,
                                                                                                                                                 y,
                                                                                                                                                 sx,
                                                                                                                                                 sy,
                                                                                                                                                 m,
                                                                                                                                                 r,
                                                                                                                                                 updateTick),
                                                                                                                                      playerId(playerId),
                                                                                                                                      ttf(ttf),
                                                                                                                                      wasCollision(false),
                                                                                                                                      danger(0.) {
    isFastCheck();
}

int PlayerPart::getTtf() const {
    return ttf;
}

void PlayerPart::isFastCheck() {
    double speed = Utils::len(dx, dy);
    updateMaxSpeed();
    is_fast = speed > maxSpeed;
}

void PlayerPart::updateVals(double x, double y, double sx, double sy, double m, double r, int ttf, int updateTick) {
    this->x = x;
    this->y = y;
    this->mass = m;
    this->radius = r;
    this->dx = sx;
    this->dy = sy;
    double speed = Utils::sqrLen(dx, dy);
    if (speed <= maxSpeed * maxSpeed) {
        is_fast = false;
    }
#ifdef LOCAL_RUN
    if (!is_fast) {
        double speed = Utils::len(dx, dy);
        if (speed > maxSpeed + 1e-9) {
            LOG_ERROR("speed > maxSpeed when nofast % > %", speed, maxSpeed);
        }
    }
#endif
    this->ttf = ttf;
    this->updateTick = updateTick;
}

void PlayerPart::recalcSpeed(double x, double y) {
    updateMaxSpeed();
    double newDx = x - this->x;
    double newDy = y - this->y;
    double speed = Utils::sqrLen(newDx, newDy);
//    if (speed > maxSpeed) {
//        if ((speed > Constants::BURST_START_SPEED + 1e-9) && (speed > Constants::SPLIT_START_SPEED + 1e-9)) {
//            this->dx /= 2; //absolutely - merge
//            this->dy /= 2;
//            return;
//        }
//    }
    this->dx = newDx;
    this->dy = newDy;
    if (speed <= maxSpeed * maxSpeed) {
        is_fast = false;
    }
}

void PlayerPart::updateVals(double x, double y, double m, double r, int updateTick) {
    this->mass = m;
    this->radius = r;
    recalcSpeed(x, y);
    this->x = x;
    this->y = y;
    this->updateTick = updateTick;
    if (this->ttf > 0) {
        --this->ttf;
    }
}

void PlayerPart::updateMaxSpeed() {
    this->maxSpeed = Constants::SPEED_FACTOR() / sqrt(mass);
}

void PlayerPart::updateVector(double aimX, double aimY) {
    if (is_fast) {
        return;
    }
    double aimDx = aimX - this->x;
    double aimDy = aimY - this->y;
    double dist = Utils::len(aimDx, aimDy);
    if (dist > 0) {
        aimDx /= dist;
        aimDy /= dist;
    } else {
        aimDx = 0;
        aimDy = 0;
    }
    dx += (aimDx * maxSpeed - dx) * Constants::INERTION_FACTOR() / mass;
    dy += (aimDy * maxSpeed - dy) * Constants::INERTION_FACTOR() / mass;
}

void PlayerPart::move() {
    double rB = x + radius, lB = x - radius;
    double dB = y + radius, uB = y - radius;

    if (rB + dx > Constants::GAME_WIDTH()) {
        x = Constants::GAME_WIDTH() - radius;
        dx = 0;
    } else if (lB + dx < 0) {
        x = radius;
        dx = 0;
    } else {
        x += dx;
    }

    if (dB + dy > Constants::GAME_HEIGHT()) {
        y = Constants::GAME_HEIGHT() - radius;
        dy = 0;
    } else if (uB + dy < 0) {
        y = radius;
        dy = 0;
    } else {
        y += dy;
    }

    if (is_fast) {
        double speed = Utils::len(dx, dy);
        is_fast = false;
        if (speed > maxSpeed) {
            if (speed - Constants::VISCOSITY() > maxSpeed) {
                double newSpeed = speed - Constants::VISCOSITY();
                dx = dx / speed * newSpeed;
                dy = dy / speed * newSpeed;
                is_fast = true;
            } else {
                dx = dx / speed * maxSpeed;
                dy = dy / speed * maxSpeed;
            }
        }
    }
    if (ttf > 0) {
        --ttf;
    }
}

void PlayerPart::collisionCalc(PlayerPart &other) {
    if (is_fast || other.is_fast) { // do not collide splits
        return;
    }
    double dist = distanceTo(other);
    if (dist >= radius + other.radius) {
        return;
    }

    // vector from centers
    double collisionVectorX = this->x - other.x;
    double collisionVectorY = this->y - other.y;
    // normalize to 1
    double vectorLen = Utils::len(collisionVectorX, collisionVectorY);
    if (vectorLen < 1e-9) { // collision object in same point??
        return;
    }
    collisionVectorX /= vectorLen;
    collisionVectorY /= vectorLen;

    double collisionForce = 1. - dist / (radius + other.radius);
    collisionForce *= collisionForce;
    collisionForce *= Constants::COLLISION_POWER;

    double sumMass = mass + other.mass;
    {
        double currPart = other.mass / sumMass;
        dx += collisionForce * currPart * collisionVectorX;
        dy += collisionForce * currPart * collisionVectorY;
        this->wasCollision = true;
    }

    {
        double otherPart = mass / sumMass;
        other.dx -= collisionForce * otherPart * collisionVectorX;
        other.dy -= collisionForce * otherPart * collisionVectorY;
        other.wasCollision = true;
    }
}

void PlayerPart::applyMyCollision(const PlayerPart &other, double &cX, double &cY) const {
    double dist = distanceTo(other);
    if (dist >= radius + other.radius) {
        return;
    }

    // vector from centers
    double collisionVectorX = this->x - other.x;
    double collisionVectorY = this->y - other.y;
    // normalize to 1
    double vectorLen = Utils::len(collisionVectorX, collisionVectorY);
    if (vectorLen < 1e-9) { // collision object in same point??
        return;
    }
    collisionVectorX /= vectorLen;
    collisionVectorY /= vectorLen;

    double collisionForce = 1. - dist / (radius + other.radius);
    collisionForce *= collisionForce;
    collisionForce *= Constants::COLLISION_POWER;

    double sumMass = mass + other.mass;
    {
        double currPart = other.mass / sumMass;
        cX += collisionForce * currPart * collisionVectorX;
        cY += collisionForce * currPart * collisionVectorY;
    }
}

void PlayerPart::shrink() {
    if (mass > Constants::MIN_SHRINK_MASS) {
        mass -= ((mass - Constants::MIN_SHRINK_MASS) * Constants::SHRINK_FACTOR);
        radius = mass2radius(mass);
    }
}

void PlayerPart::eat(const Food &food) {
    mass += food.getMass();
}

void PlayerPart::eat(const GameObject &food) {
    mass += food.getMass();
}

void PlayerPart::updateByMass() {
    if (mass != startMass) {
        updateMaxSpeed();

        double new_radius = mass2radius(mass);
        if (radius != new_radius) {
            radius = new_radius;
        }
        if (!is_fast) {
            double currSpeed = Utils::len(dx, dy);
            if (currSpeed > maxSpeed) {
                double mul = maxSpeed / currSpeed;
                dx *= mul;
                dy *= mul;
            }
        }

        if (x < radius) {
            x = radius;
        } else if (x + radius > Constants::GAME_WIDTH()) {
            x = Constants::GAME_WIDTH() - radius;
        }
        if (y < radius) {
            y = radius;
        } else if (y + radius > Constants::GAME_HEIGHT()) {
            y = Constants::GAME_HEIGHT() - radius;
        }
    }
}

double PlayerPart::getStartMass() const {
    return startMass;
}

void PlayerPart::updateStartMass() {
    this->startMass = mass;
}

double PlayerPart::getMaxSpeed() const {
    return maxSpeed;
}

bool PlayerPart::fuse(const PlayerPart &other) {
    if (ttf != 0 || other.ttf != 0) {
        return false;
    }
    double dist = distanceTo(other);
    double nR = radius + other.radius;
    if (dist > nR) {
        return false;
    }

    double sumMass = mass + other.mass;

    double fragInfluence = other.mass / sumMass;
    double currInfluence = mass / sumMass;

    // center with both parts influence
    this->x = this->x * currInfluence + other.x * fragInfluence;
    this->y = this->y * currInfluence + other.y * fragInfluence;

    // new move vector with both parts influence
    dx = dx * currInfluence + other.dx * fragInfluence;
    dy = dy * currInfluence + other.dy * fragInfluence;

    mass += other.mass;
    updateMaxSpeed();
    return true;
}

void PlayerPart::addMass(double mass) {
    this->mass += mass;
}

void PlayerPart::setImpulse(double speed, double angle) {
    dx = cos(angle) * speed;
    dy = sin(angle) * speed;
    is_fast = speed > maxSpeed;
}

void PlayerPart::burstUpdate(int newId, double newMass, double newRadius) {
    this->id = newId;
    this->mass = newMass;
    this->radius = newRadius;
    this->ttf = Constants::TICKS_TIL_FUSION();
}

void PlayerPart::updateTTF(int ttf) {
    this->ttf = ttf;
}

void PlayerPart::setIsFast() {
    this->is_fast = true;
}

double PlayerPart::getDanger() const {
    return danger;
}

void PlayerPart::updateDanger(double newValue) {
    this->danger = std::max(this->danger, newValue);
}

void PlayerPart::updateNextTick(PlayerPart &part) {
    this->x = part.x;
    this->y = part.y;
    this->dx = part.dx;
    this->dy = part.dy;
    this->mass = part.mass;
    this->radius = part.radius;

    this->ttf = part.ttf;
    this->is_fast = part.is_fast;
}
