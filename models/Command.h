//
// Created by dragoon on 3/28/18.
//

#ifndef ALMOSTAGARIO_COMMAND_H
#define ALMOSTAGARIO_COMMAND_H

#include <string>
#include <vector>
#include "../../nlohmann/json.hpp"

using json = nlohmann::json;

class Command {
    double x, y;
    std::string debug = "";
    bool split;
    bool eject;
    std::vector<std::pair<std::string, std::string> > sprites;

public:
    Command(double x, double y);

    Command(double x, double y, bool split, bool eject);

    Command(double x, double y, const std::string &debug);

    double getX() const;

    double getY() const;

    void appendDebug(std::string &append);

    void appendDebug(std::string append);

    void doSplit();

    void doEject();

    void addSprite(std::string &id, std::string &text);

    json toJson();
};


#endif //ALMOSTAGARIO_COMMAND_H
