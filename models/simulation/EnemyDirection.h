//
// Created by dragoon on 4/12/18.
//

#ifndef ALMOSTAGARIO_ENEMYDIRECTION_H
#define ALMOSTAGARIO_ENEMYDIRECTION_H


struct EnemyDirection {
    double x1, y1, x2, y2;
    bool exact;

    EnemyDirection(double x1, double y1, double x2, double y2, bool exact);
};


#endif //ALMOSTAGARIO_ENEMYDIRECTION_H
