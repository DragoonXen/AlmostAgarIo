//
// Created by dragoon on 3/28/18.
//

#include "Command.h"

Command::Command(double x, double y) : x(x), y(y), split(false), eject(false) {}

Command::Command(double x, double y, const std::string &debug) : x(x), y(y), split(false), eject(false) {
    this->debug = debug;
}

void Command::appendDebug(std::string &append) {
    this->debug += append;
}

void Command::appendDebug(std::string append) {
    this->debug += append;
}

void Command::doSplit() {
    this->split = true;
}

void Command::doEject() {
    this->eject = true;
}

void Command::addSprite(std::string &id, std::string &text) {
    this->sprites.emplace_back(id, text);
}

json Command::toJson() {
    json j;
    j["X"] = x;
    j["Y"] = y;
    if (!debug.empty()) {
        j["Debug"] = debug;
    }
    if (split) {
        j["Split"] = true;
    }
    if (eject) {
        j["Eject"] = true;
    }
//    if (!sprites.empty()) {
//
//    }
    return j;
}

double Command::getX() const {
    return x;
}

double Command::getY() const {
    return y;
}

Command::Command(double x, double y, bool split, bool eject) : x(x), y(y), split(split), eject(eject) {}

