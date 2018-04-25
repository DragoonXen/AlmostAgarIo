//
// Created by dragoon on 4/8/18.
//

#ifndef ALMOSTAGARIO_SIMSTATE_H
#define ALMOSTAGARIO_SIMSTATE_H


#include <vector>
#include "simulation/SimCommand.h"
#include "../Constants.h"
#include "simulation/SimulateType.h"

class SimState {
public:
    std::vector<std::pair<SimCommand, int>> simCommands;
    double score;
    SimulateType simulateType;

    SimState(std::vector<std::pair<SimCommand, int>> &simCommands, std::pair<double, SimulateType> score)
            : simCommands(std::vector<std::pair<SimCommand, int>>()),
              score(score.first),
              simulateType(score.second) {
        this->simCommands.swap(simCommands);
    }

    void updateValues(std::vector<std::pair<SimCommand, int>> &simCommands, std::pair<double, SimulateType> score) {
        if (score.first > this->score) {
            this->simCommands.swap(simCommands);
            this->score = score.first;
            this->simulateType = score.second;
        }
        simCommands.clear();
    }

    void addBasicBonus() {
        this->score += Constants::ADD_SCORE_FOR_PREV;
    }

    void swap(SimState &other) {
        {
            double tmp = score;
            score = other.score;
            other.score = tmp;
        }
        {
            auto tmp = simulateType;
            simulateType = other.simulateType;
            other.simulateType = tmp;
        }
        simCommands.swap(other.simCommands);
    }
};


#endif //ALMOSTAGARIO_SIMSTATE_H
