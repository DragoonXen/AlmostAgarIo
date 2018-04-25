//
// Created by dragoon on 3/28/18.
//

#ifndef ALMOSTAGARIO_UTILS_H
#define ALMOSTAGARIO_UTILS_H

#include "models/Food.h"
#include "models/GameObject.h"

class Utils {

public:
    inline static double distSqr(double x, double y, double x2, double y2) {
        return (x - x2) * (x - x2) + (y - y2) * (y - y2);
    }

    inline static double dist(double x, double y, double x2, double y2) {
        return sqrt(distSqr(x, y, x2, y2));
    }

    inline static double len(double x, double y) {
        return sqrt(x * x + y * y);
    }

    inline static double sqrLen(double x, double y) {
        return x * x + y * y;
    }

    inline static void toWall(double sX, double sY, double &aX, double &aY, double basicLen) {
        double mulFirst = 2000. / basicLen;
        aX *= mulFirst;
        aY *= mulFirst;
        double new_x = sX + aX;
        double new_y = sY + aY;
        double mul = 1.;
        if (new_x < 0) {
            mul = std::min(mul, -sX / aX);
        }
        if (new_x > Constants::GAME_WIDTH()) {
            mul = std::min(mul, (Constants::GAME_WIDTH() - sX) / aX);
        }
        if (new_y < 0) {
            mul = std::min(mul, -sY / aY);
        }
        if (new_y > Constants::GAME_HEIGHT()) {
            mul = std::min(mul, (Constants::GAME_HEIGHT() - sY) / aY);
        }
        aX *= mul;
        aY *= mul;
    }

    static void splitPlayerId(std::string id, int &first, int &second) {
        if (id[1] == '.') {
            id[1] = ' ';
            std::stringstream ss(id);
            ss >> first >> second;
            --first;
        } else {
            first = std::stoi(id);
            --first;
            second = 0;
        }
    }

    static GameObject partForCornerCheck;

    static bool bannedFood(const Food &food, double rad) {
        const double &x = food.getX();
        const double &y = food.getY();
        if (x < rad) { // too left
            if (y < rad) { // too top
                partForCornerCheck.updateCoordsAndRad(rad, rad, rad);
                return !partForCornerCheck.checkEating(food);
            } else if (y > Constants::GAME_HEIGHT() - rad) {// too bottom
                partForCornerCheck.updateCoordsAndRad(rad, Constants::GAME_HEIGHT() - rad, rad);
                return !partForCornerCheck.checkEating(food);
            }
        } else if (x > Constants::GAME_WIDTH() - rad) { // too right
            if (y < rad) { // too top
                partForCornerCheck.updateCoordsAndRad(Constants::GAME_WIDTH() - rad, rad, rad);
                return !partForCornerCheck.checkEating(food);
            } else if (y > Constants::GAME_HEIGHT() - rad) { // too bottom
                partForCornerCheck.updateCoordsAndRad(Constants::GAME_WIDTH() - rad, Constants::GAME_HEIGHT() - rad, rad);
                return !partForCornerCheck.checkEating(food);
            }
        }
        return false;
    }
};


#endif //ALMOSTAGARIO_UTILS_H
