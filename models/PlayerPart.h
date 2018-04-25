//
// Created by dragoon on 3/27/18.
//

#ifndef ALMOSTAGARIO_PLAYER_PART_H
#define ALMOSTAGARIO_PLAYER_PART_H


#include "GameObject.h"
#include "../Utils.h"

class PlayerPart : public GameObject {

protected:
    int playerId;
    int ttf;
    bool is_fast;
    double maxSpeed;
    bool wasCollision;
    double startMass;
    double danger;

public:

    PlayerPart(int id, int playerId, double x, double y, double sx, double sy, double m, double r, int updateTick, int ttf);

    void updateStartMass();

    double getStartMass() const;

    void updateVector(double aimX, double aimY);

    void collisionCalc(PlayerPart &other);

    void applyMyCollision(const PlayerPart &other, double &cX, double &cY) const;

    void move();

    int getTtf() const;

    inline int getPlayerId() const {
        return playerId;
    }

    void recalcSpeed(double x, double y);

    void updateVals(double x, double y, double m, double r, int updateTick);

    void updateVals(double x, double y, double sx, double sy, double m, double r, int ttf, int updateTick);

    void updateMaxSpeed();

    double getMaxSpeed() const;

    void shrink();

    void eat(const Food &food);

    void eat(const GameObject &food);

    void updateByMass();

    void isFastCheck();

    bool fuse(const PlayerPart &other);

    inline bool canBurst(size_t yetCnt) const {
        return Constants::MAX_FRAGS_CNT() > yetCnt && mass > Constants::MIN_BURST_MASS * 2;
    }

    void addMass(double mass);

    void setImpulse(double speed, double angle);

    void updateTTF(int ttf);

    void setIsFast();

    inline bool isFast() const {
        return is_fast;
    }

    double getDanger() const;

    void updateDanger(double newValue);

    void burstUpdate(int newId, double newMass, double newRadius);

    void updateNextTick(PlayerPart &part);

    inline void normalizeSpeed() {
        if (dx * dx + dy * dy > maxSpeed * maxSpeed) {
            double norm = maxSpeed / Utils::len(dx, dy);
            dx *= norm;
            dy *= norm;
        }
    }

    void fixSpeedAfterCollision() {
        if (wasCollision) {
            normalizeSpeed();
            wasCollision = false;
        }
    }

    inline bool canSplit(int currentCnt) const {
        return currentCnt < Constants::MAX_FRAGS_CNT() && Constants::MIN_SPLIT_MASS < mass;
    }

    inline static double mass2radius(double mass) {
        return Constants::RADIUS_FACTOR * std::sqrt(mass);
    }
};


#endif //ALMOSTAGARIO_PLAYER_PART_H
