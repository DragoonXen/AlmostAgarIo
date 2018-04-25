//
// Created by dragoon on 4/15/18.
//

#include "KnownFood.h"

int KnownFood::totalCnt = 0;
KnownFood::KnownFood(int x, int y) : x(x), y(y) {
    ++totalCnt;
}