//
// Created by dragoon on 4/2/18.
//

#ifndef ALMOSTAGARIO_SIMCOMMAND_H
#define ALMOSTAGARIO_SIMCOMMAND_H


#include "../PlayerPart.h"
#include "../../Context.h"

class SimCommand {
public:
    double x, y;
    int userId, fragmentId;
    bool split, eject;

    explicit SimCommand(const PlayerPart &target);

    SimCommand(const PlayerPart &target, bool split);

    SimCommand(const PlayerPart &target, bool split, bool eject);

    SimCommand(double x, double y);

    SimCommand(double x, double y, bool split);

    SimCommand(double x, double y, bool split, bool eject);

    inline void updateXY(const Context &context) {
        for (auto &part : context.players[userId].parts) {
            if (part.getId() == fragmentId) {
                x = part.getX();
                y = part.getY();
                return;
            }
        }
    }

};


#endif //ALMOSTAGARIO_SIMCOMMAND_H
