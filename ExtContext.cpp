//
// Created by dragoon on 4/7/18.
//

#include "ExtContext.h"

ExtContext::ExtContext(int tickIndex) : Context(tickIndex),
                                        simTicks(std::list<int>()),
                                        turnTime(std::list<int>()),
                                        totalTicks(0),
                                        totalTurnTime(0),
                                        lastSimCount(0),
                                        totalLength(0L),
                                        tlPrevention(0L),
                                        disableAddPoints(false),
                                        ttf(std::map<std::pair<int, int>, int>()),
                                        suspicious(std::set<std::pair<int, int>>()),
                                        nextStepSuspicious(std::set<std::pair<int, int>>()) {
}

void ExtContext::updateTtfTick() {
    std::swap(this->nextStepSuspicious, this->suspicious);
    this->nextStepSuspicious.clear();
    for (auto iter = ttf.begin(); iter != ttf.end();) {
        if (iter->second == 1) {
            iter = ttf.erase(iter);
        } else {
            --iter->second;
            ++iter;
        }
    }
}
