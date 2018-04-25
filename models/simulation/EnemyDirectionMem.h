//
// Created by dragoon on 4/12/18.
//

#ifndef ALMOSTAGARIO_ENEMYDIRECTIONMEM_H
#define ALMOSTAGARIO_ENEMYDIRECTIONMEM_H


struct EnemyDirectionMem {
    int fragId;
    double x, y;
    double dx, dy;
    double cX, cY;
    double mass;
    bool exact;

    EnemyDirectionMem(int fragId, double x, double y, double dx, double dy, double cX, double cY, double mass, bool exact);
};


#endif //ALMOSTAGARIO_ENEMYDIRECTIONMEM_H
