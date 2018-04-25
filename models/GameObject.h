//
// Created by dragoon on 3/27/18.
//

#ifndef ALMOSTAGARIO_GAMEOBJECT_H
#define ALMOSTAGARIO_GAMEOBJECT_H

#include <string>
#include <cmath>
#include "../Constants.h"
#include "Food.h"

class GameObject {
protected:
    int id;
    double x, y, dx, dy;
    double mass, radius;
    int updateTick;

public:
    GameObject(int id, double x, double y, double sx, double sy, double m, double r, int updateTick);

    int getId() const;

    double getX() const;

    double getY() const;

    double getDx() const;

    double getDy() const;

    inline double getMass() const {
        return mass;
    }

    inline double getRadius() const {
        return radius;
    }

    int getUpdateTick() const;

    inline void updateCoordsAndRad(double x, double y, double rad) {
        this->x = x;
        this->y = y;
        this->radius = rad;
    }

    bool checkEating(const Food &food) const {
        double distance = sqrDist(food);
        double coverDist = radius - food.getRadius() * Constants::EATING_RAD;
        return coverDist * coverDist > distance;
    }

    inline double checkEatingDist(const Food &food) const {
        double distance = sqrDist(food);
        double coverDist = radius - food.getRadius() * Constants::EATING_RAD;
        if (coverDist * coverDist > distance) {
            return radius - sqrt(distance);
        }
        return 0.;
    }

    inline double checkEatingDist(const GameObject &food) const {
        double distance = sqrDist(food);
        double coverDist = radius - food.getRadius() * Constants::EATING_RAD;
        if (coverDist * coverDist > distance) {
            return radius - sqrt(distance);
        }
        return 0.;
    }

    inline double distanceTo(const Food &other) const {
        return std::sqrt(sqrDist(other));
    }

    inline double distanceTo(const GameObject &other) const {
        return std::sqrt(sqrDist(other));
    }

    inline double sqrDist(const Food &other) const {
        return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
    }

    inline double sqrDist(const GameObject &other) const {
        return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
    }

    inline double sqrDist(double x, double y) const {
        return (x - this->x) * (x - this->x) + (y - this->y) * (y - this->y);
    }
};


#endif //ALMOSTAGARIO_GAMEOBJECT_H
