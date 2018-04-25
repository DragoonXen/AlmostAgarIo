//
// Created by dragoon on 3/27/18.
//

#include "Context.h"
#include "Constants.h"

int Context::getTickIndex() const {
    return tickIndex;
}

void Context::nextTick(double currDestX, double currDestY) {
    ++this->tickIndex;
    this->prevDestX = currDestX;
    this->prevDestY = currDestY;
}

double Context::getPrevDestX() const {
    return prevDestX;
}

double Context::getPrevDestY() const {
    return prevDestY;
}

Context::Context(int tickIndex) : tickIndex(tickIndex), players{{Player(0), Player(1), Player(2), Player(3)}}, me(players.data()) {
    this->dummy = true;
    this->prevDestX = Constants::GAME_WIDTH() / 2.f;
    this->prevDestY = Constants::GAME_HEIGHT() / 2.f;
}

void Context::updatePlayerParts() {
    allPlayersParts.clear();
    for (auto &player : players) {
        for (auto &playerPart : player.parts) {
            allPlayersParts.push_back(&playerPart);
        }
    }
}
