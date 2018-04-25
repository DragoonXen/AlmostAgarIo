//
// Created by dragoon on 4/15/18.
//

#ifndef ALMOSTAGARIO_KNOWNFOOD_H
#define ALMOSTAGARIO_KNOWNFOOD_H

struct KnownFood {
    int x, y;

    static int totalCnt;

    KnownFood(int x, int y);

    void del() {
        --totalCnt;
    }
};


#endif //ALMOSTAGARIO_KNOWNFOOD_H
