//
// Created by dragoon on 4/2/18.
//

#include "SimCommand.h"

SimCommand::SimCommand(double x, double y) : x(x), y(y), userId(-1), fragmentId(-1), split(false), eject(false) {}

SimCommand::SimCommand(double x, double y, bool split) : x(x), y(y), userId(-1), fragmentId(-1), split(split), eject(false) {}

SimCommand::SimCommand(double x, double y, bool split, bool eject) : x(x), y(y), userId(-1), fragmentId(-1), split(split), eject(eject) {}

SimCommand::SimCommand(const PlayerPart &target) : x(target.getX()), y(target.getY()), userId(target.getPlayerId()), fragmentId(target.getId()), split(false), eject(false) {}

SimCommand::SimCommand(const PlayerPart &target, bool split) : x(target.getX()), y(target.getY()), userId(target.getPlayerId()), fragmentId(target.getId()), split(split), eject(false)  {}

SimCommand::SimCommand(const PlayerPart &target, bool split, bool eject) : x(target.getX()), y(target.getY()), userId(target.getPlayerId()), fragmentId(target.getId()), split(split), eject(eject) {}