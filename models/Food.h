//
// Created by dragoon on 3/27/18.
//

#ifndef ALMOSTAGARIO_FOOD_H
#define ALMOSTAGARIO_FOOD_H


#include "../Constants.h"

class Food {
public:
    double x, y;

    Food(double x, double y);

    inline double getMass() const {
        return Constants::FOOD_MASS();
    }

    inline double getRadius() const {
        return Constants::FOOD_RADIUS;
    }

    inline double getX() const {
        return x;
    }

    inline double getY() const {
        return y;
    }
};


#endif //ALMOSTAGARIO_FOOD_H
