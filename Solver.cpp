//
// Created by dragoon on 3/27/18.
//
#ifdef REWIND_VIEWER

#include "../rewind/RewindClient.h"

#endif

#include "Solver.h"
#include "SimpleLog.h"
#include "Utils.h"
#include "InputParser.h"
#include "models/simulation/SimCommand.h"
#include "models/simulation/Simulator.h"
#include "models/SimState.h"

#define _USE_MATH_DEFINES

#include <math.h>

inline bool isNear(const ExtContext &context) {
    for (auto &player : context.me->parts) {
        if (Utils::dist(context.getPrevDestX(), context.getPrevDestY(), player.getX(), player.getY()) <= player.getRadius() * 1.2) {
            return true;
        }
    }
    return false;
}

inline bool meCanSplit(const ExtContext &context) {
    if (Constants::MAX_FRAGS_CNT() == context.me->parts.size()) {
        return false;
    }
    for (auto &playerPart : context.me->parts) {
        if (playerPart.getMass() > Constants::MIN_SPLIT_MASS) {
            return true;
        }
    }
    return false;
}

inline bool checkMinMaxTTF(const ExtContext &context) {
    for (auto &player : context.players) {
        if (&player == context.me) {
            continue;
        }
        if (player.parts.empty()) {
            continue;
        }
        if (player.parts.size() == 1 && player.parts.front().getId() == 0) { // solid players has exactly 0 ttf
            continue;
        }
        for (auto &part : player.parts) {
            auto pair = std::make_pair(part.getPlayerId(), part.getId());
            if (context.ttf.find(pair) == context.ttf.end()) { // no data for player, simulate both
                return true;
            }
        }
    }
    return false;
}

inline Context *maxContextTtf(const ExtContext &context) {
    auto *result = new Context(context);
    Context &contextCopy = *result;
    for (auto &player : contextCopy.players) {
        if (&player == contextCopy.me) { // no need in changes for me
            continue;
        }
        if (player.parts.empty()) {
            continue;
        }
        if (player.parts.size() == 1 && player.parts.front().getId() == 0) {
            continue;
        }
        for (auto &playerPart : player.parts) {
            auto pair = std::make_pair(playerPart.getPlayerId(), playerPart.getId());
            auto iter = context.ttf.lower_bound(pair);
            if (iter == context.ttf.end() || iter->first.first != pair.first) { // not found any greater id for this player - max ttf
                playerPart.updateTTF(Constants::TICKS_TIL_FUSION());
                continue;
            }
            if (iter->first.first == playerPart.getPlayerId() &&
                iter->first.second == playerPart.getId()) {// found exactly this player - no need in modifications
                continue;
            }
            playerPart.updateTTF(iter->second); // new ttf - ttf for next id for this player
        }
    }
    return result;
}

inline int tlPrevention(ExtContext &context, int simulateTicks) {
#ifndef DISABLE_TL_PREVENTION
    context.tlPrevention = 0;
    context.disableAddPoints = false;
    if (context.turnTime.empty()) {
        return simulateTicks;
    }
    context.totalTicks += context.simTicks.back();
    context.totalTurnTime += context.turnTime.back();
    if (context.turnTime.size() < Constants::MA_PERIOD) {
        return simulateTicks;
    }
    context.totalTicks -= context.simTicks.front();
    context.totalTurnTime -= context.turnTime.front();

    context.turnTime.pop_front();
    context.simTicks.pop_front();

    auto microsPerTick = (int) (context.totalLength / context.tickIndex);
    if (microsPerTick * 4 / 3 < Constants::TL) {
        return simulateTicks;
    }
    //same as above for remaining time
    long remainedTime = Constants::TL * Constants::GAME_TICKS() - context.totalLength;
    int ticksRemained = Constants::GAME_TICKS() - context.getTickIndex();
    double timePerTick = remainedTime / (double) ticksRemained;
    if (Constants::TL * 4 / 3 < timePerTick) {
        return simulateTicks;
    }
    context.disableAddPoints = true;

    double timePerSim = context.totalTurnTime / (double) context.totalTicks;
    auto allowedSimCnt = (int) floor(timePerTick / timePerSim);
    if (context.lastSimCount > allowedSimCnt) {
        allowedSimCnt = context.lastSimCount - 1;
    }

    allowedSimCnt = std::max(10, allowedSimCnt);
    if (simulateTicks > allowedSimCnt) {
        context.tlPrevention = simulateTicks - allowedSimCnt;
        simulateTicks = allowedSimCnt;
    }
    return simulateTicks;
#else
    while (!context.turnTime.empty()){
        context.turnTime.pop_front();
    }
    while (!context.simTicks.empty()){
        context.simTicks.pop_front();
    }
    return simulateTicks;
#endif
}

void removeSplit(std::vector<std::pair<SimCommand, int>> &simCommands) {
    auto it = simCommands.begin();
    while (true) {
        auto it2 = std::next(it);
        if (it2 == simCommands.end()) {
            return;
        }
        if (it->first.x == it2->first.x && it->first.y == it2->first.y) {
            it->second += it2->second;
            it->first.split = false;
            simCommands.erase(it2);
        } else {
            ++it;
        }
    }
}

inline Command
applySelectedSimStateAsCommand(SimState &selectedScore, ExtContext &extContext, Context const *const maxTtfContext, int simulateTicks) {
#ifdef REWIND_VIEWER
    LOG_DEBUG("% % sim type",
              (selectedScore.simulateType == SimulateType::MAX_TTF_DETECT || selectedScore.simulateType == SimulateType::MIN_TTF_DETECT) ? "detect"
                                                                                                                                         : "nodetect",
              (selectedScore.simulateType == SimulateType::MAX_TTF_DETECT || selectedScore.simulateType == SimulateType::MAX_TTF_NODETECT)
              ? "maxTTF"
              : "minTTF");
    Simulator::debugSimulateProxy(extContext, selectedScore.simCommands, simulateTicks, maxTtfContext, selectedScore.simulateType);
#endif
    selectedScore.simCommands.swap(extContext.choosedCommands);
    auto &bestDxDy = extContext.choosedCommands[0];
    return Command(bestDxDy.first.x, bestDxDy.first.y, bestDxDy.first.split, false); //3399
}

Command Solver::onTick(ExtContext &context, json &input) {
    InputParser::parse(context, input);

#ifdef REWIND_VIEWER
    {
        uint32_t playerColor[] = {0xFFFF00, 0xFF00FF, RewindClient::COLOR_BLUE, RewindClient::COLOR_GREEN, 0x00FFFF};
        RewindClient &client = RewindClient::instance();
        for (auto &player : context.me->parts) {
            client.circle(player.getX(), player.getY(), player.getRadius(), playerColor[context.me->id], 4);
            client.line(player.getX(), player.getY(), player.getX() + player.getDx() * 10, player.getY() + player.getDy() * 10, 0xFF0000, 4);

            double visShiftNorm = Constants::VIS_SHIFT / Utils::len(player.getDx(), player.getDy());
            double xVisionCenter = player.getX() + player.getDx() * visShiftNorm;
            double yVisionCenter = player.getY() + player.getDy() * visShiftNorm;

            double visionR;
            if (context.me->parts.size() == 1) {
                visionR = player.getRadius() * Constants::VIS_FACTOR;
            } else {
                visionR = player.getRadius() * Constants::VIS_FACTOR_FR * sqrt(context.me->parts.size());
            }
            client.circle(xVisionCenter, yVisionCenter, visionR, 0xaaFFFFFF, 2);
        }
        for (auto &player : context.players) {
//            LOG_DEBUG("%", player.getPlayerId());
            if (&player == context.me) {
                continue;
            }
            for (auto &playerPart : player.parts) {
                client.circle(playerPart.getX(), playerPart.getY(), playerPart.getRadius(), playerColor[player.id], 3);
                client.line(playerPart.getX(),
                            playerPart.getY(),
                            playerPart.getX() + playerPart.getDx() * 10,
                            playerPart.getY() + playerPart.getDy() * 10,
                            0xFF0000, 3);
            }
        }

        for (auto &food : context.viruses) {
            client.circle(food.getX(), food.getY(), food.getRadius(), 0, 3);
        }
        for (auto &food : context.ejections) {
            client.circle(food.getX(), food.getY(), food.getRadius(), 0xFF8800, 3);
        }
        for (auto &food : context.food) {
            client.circle(food.getX(), food.getY(), food.getRadius(), 0xB00000, 3);
        }
    }
#endif
    auto &me = context.me;
    if (me->parts.empty()) {
        return Command(context.getPrevDestX(), context.getPrevDestY(), "dead");
    }
    auto &food = context.food;

    double midX = 0.;
    double midY = 0.;
    for (auto &part : me->parts) {
        midX += part.getX();
        midY += part.getY();
    }
    midX /= me->parts.size();
    midY /= me->parts.size();

    bool simulate = !food.empty();
    simulate |= !context.ejections.empty();
    bool enemySpotted = false;
    for (auto &player : context.players) {
        if (&player == context.me) {
            continue;
        }
        if (!player.parts.empty()) {
            enemySpotted = true;
            simulate = true;
            break;
        }
    }
    simulate |= (!context.viruses.empty());
    if (simulate) {
        bool testSplit = meCanSplit(context);
        std::unique_ptr<Context> copy = checkMinMaxTTF(context) ? std::unique_ptr<Context>(maxContextTtf(context)) : nullptr;
        int simulateTicks = 50;
        double avgMaxSpeed = 0.;
        double avgSpeed = 0.;
        double avgMass = 0.;
        for (auto &playerPart : context.me->parts) {
            avgMaxSpeed += playerPart.getMaxSpeed();
            avgSpeed += Utils::len(playerPart.getDx(), playerPart.getDy());
            avgMass += playerPart.getMass();
        }
        avgMaxSpeed /= context.me->parts.size();
        avgSpeed /= context.me->parts.size();
        avgMass /= context.me->parts.size();
        int ticksForDistance = 0;
        double aimDistance = Constants::GAME_WIDTH() * .25;
        {
            double inertia = Constants::INERTION_FACTOR() / avgMass;
            while (aimDistance > 0.) {
                ++ticksForDistance;
                aimDistance -= avgSpeed;
                avgSpeed += (avgMaxSpeed - avgSpeed) * inertia;
            }
        }
        simulateTicks = std::min(ticksForDistance, simulateTicks);
        simulateTicks = tlPrevention(context, simulateTicks);
        context.simTicks.push_back(simulateTicks);
        context.lastSimCount = simulateTicks;

        if (!context.choosedCommands.empty()) {
            auto &commands = context.choosedCommands;
            if (commands[0].second == 1) {
                commands.erase(commands.begin());
            } else {
                --commands[0].second;
            }
        }

        std::vector<std::pair<SimCommand, int>> commands;
        bool choosedCommands = context.choosedCommands.size() > 1;
        if (choosedCommands) {
            commands.swap(context.choosedCommands);
            if (commands[0].first.userId != -1) {
                commands[0].first.updateXY(context);
            }
            //prepare for this step
            int ticks = 0;
            for (auto &comm : commands) {
                ticks += comm.second;
            }
            if (ticks < simulateTicks) { // add ticks
                auto &last = commands.back();
                if (last.first.eject || last.first.split) {
                    commands.emplace_back(last.first, simulateTicks - ticks);
                    commands.back().first.eject = false;
                    commands.back().first.split = false;
                } else {
                    last.second += simulateTicks - ticks;
                }
            } else {
                while (ticks > simulateTicks) { // remove ticks
                    if (commands.back().second <= ticks - simulateTicks) {
                        ticks -= commands.back().second;
                        commands.pop_back();
                    } else {
                        commands.back().second -= (ticks - simulateTicks);
                        ticks = simulateTicks;
                    }
                }
            }
        } else {
            commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), simulateTicks);
        }
        SimState basicSimState(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), -1e100));
        if (testSplit && !choosedCommands) {
            commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY(), true), 1);
            commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), simulateTicks - 1);
            basicSimState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), -1e100));
            commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), Constants::SPLIT_DELAY);
            commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY(), true), 1);
            commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), simulateTicks - 1 - Constants::SPLIT_DELAY);
            basicSimState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), -1e100));
        }
//        if (!context.food.empty() || enemySpotted) {
            basicSimState.addBasicBonus();
//        }
        double maxScore = basicSimState.score;
        SimState *bestSimState = &basicSimState;


        std::vector<SimState> scores;
        {
            const int angleChange = 15;
            scores.reserve(360 / angleChange * 2);
            for (int i = 0; i != 360; i += angleChange) { // every 15 degrees
                double angle = i * M_PI / 180.;
                double dx = cos(angle);
                double dy = sin(angle);
                Utils::toWall(midX, midY, dx, dy, 1.);
                double aimX = dx + midX;
                double aimY = dy + midY;
                aimX = std::min(std::max(0., aimX), (double) Constants::GAME_WIDTH());
                aimY = std::min(std::max(0., aimY), (double) Constants::GAME_HEIGHT());

                commands.emplace_back(SimCommand(aimX, aimY), simulateTicks);
                SimState simState(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                if (testSplit) {
                    commands.emplace_back(SimCommand(aimX, aimY, true), 1);
                    commands.emplace_back(SimCommand(aimX, aimY), simulateTicks - 1);
                    simState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                    commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), Constants::SPLIT_DELAY);
                    commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY(), true), 1);
                    commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), simulateTicks - 1 - Constants::SPLIT_DELAY);
                    simState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                }
                scores.push_back(simState);
                if (maxScore < simState.score) {
                    maxScore = simState.score;
                    bestSimState = &scores.back();
                }
            }
        }

        std::shared_ptr<SimState> aimState;
        // central point
        {
            commands.emplace_back(SimCommand(midX, midY), simulateTicks);
            SimState simState(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
            if (testSplit) {
                commands.emplace_back(SimCommand(midX, midY, true), 1);
                commands.emplace_back(SimCommand(midX, midY), simulateTicks - 1);
                simState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), Constants::SPLIT_DELAY);
                commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY(), true), 1);
                commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), simulateTicks - 1 - Constants::SPLIT_DELAY);
                simState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
            }
            if (maxScore < simState.score) {
                maxScore = simState.score;
                aimState = std::make_shared<SimState>(simState);
                bestSimState = aimState.get();
            }
        }
        // disable this part of logic, if we can overflow TL
        if (!context.disableAddPoints) {
            // other player parts as aims
            for (auto &player : context.players) {
                if (&player == context.me) {
                    continue;
                }
                for (auto &part : player.parts) {
                    commands.emplace_back(SimCommand(part), simulateTicks);
                    SimState simState(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                    if (testSplit) {
                        commands.emplace_back(SimCommand(part, true), 1);
                        commands.emplace_back(SimCommand(part), simulateTicks - 1);
                        simState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                        commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), Constants::SPLIT_DELAY);
                        commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY(), true), 1);
                        commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), simulateTicks - 1 - Constants::SPLIT_DELAY);
                        simState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                    }
                    if (maxScore < simState.score) {
                        maxScore = simState.score;
                        aimState = std::make_shared<SimState>(simState);
                        bestSimState = aimState.get();
                    }
                }
            }

            //near central point
            if (context.me->parts.size() > 1) {

                double avgLength = 0.;
                for (auto &part : context.me->parts) {
                    avgLength += Utils::dist(midX, midY, part.getX(), part.getY());
                }
                avgLength /= context.me->parts.size();
                if (avgLength > 10.) {
                    const int angleChange = 45;
                    for (int i = 0; i != 360; i += angleChange) { // every 45 degrees
                        double angle = i * M_PI / 180.;
                        double dx = cos(angle);
                        double dy = sin(angle);
                        Utils::toWall(midX, midY, dx, dy, 2000. / avgLength);
                        double aimX = dx + midX;
                        double aimY = dy + midY;
                        aimX = std::min(std::max(0., aimX), (double) Constants::GAME_WIDTH());
                        aimY = std::min(std::max(0., aimY), (double) Constants::GAME_HEIGHT());

                        commands.emplace_back(SimCommand(aimX, aimY), simulateTicks);
                        SimState simState(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                        if (testSplit) {
                            commands.emplace_back(SimCommand(aimX, aimY, true), 1);
                            commands.emplace_back(SimCommand(aimX, aimY), simulateTicks - 1);
                            simState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                            commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), Constants::SPLIT_DELAY);
                            commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY(), true), 1);
                            commands.emplace_back(SimCommand(context.getPrevDestX(), context.getPrevDestY()), simulateTicks - 1 - Constants::SPLIT_DELAY);
                            simState.updateValues(commands, Simulator::simulateProxy(context, commands, simulateTicks, copy.get(), maxScore));
                        }
                        if (maxScore < simState.score) {
                            maxScore = simState.score;
                            aimState = std::make_shared<SimState>(simState);
                            bestSimState = aimState.get();
                        }
                    }
                }
            }
        }

        bool applySimulation = (basicSimState.simCommands.size() > 1 || maxScore > basicSimState.score);
        if (!applySimulation) {
            for (int i = 0; i != scores.size(); ++i) {
                if (scores[i].score != basicSimState.score) {
                    applySimulation = true;
                    break;
                }
            }
        }
        if (applySimulation) {
            double maxFound = scores[0].score;
            for (size_t i = 0; i != scores.size(); ++i) {
                maxFound = std::max(maxFound, scores[i].score);
            }

            // first - long directions
            if (maxFound == maxScore) {
                size_t currMax = 0;
                size_t currStart = 0;
                size_t maxStart = 0;
                size_t maxLen = 0;
                for (size_t idx = 0; idx != (scores.size() * 2); ++idx) { // cycle check
                    size_t i = idx % scores.size();
                    if (scores[i].score == maxFound) {
                        ++currMax;
                    } else {
                        if (currMax > maxLen) {
                            maxLen = currMax;
                            maxStart = currStart;
                        }
                        currStart = idx + 1;
                        currMax = 0;
                    }
                }
                if (currMax > maxLen) {
                    maxLen = currMax;
                    maxStart = currStart;
                }
                size_t selected = maxStart + (maxLen + 1) / 2 - 1;
                if (selected >= scores.size()) {
                    selected -= scores.size();
                }

                return applySelectedSimStateAsCommand(scores[selected], context, copy.get(), simulateTicks);
            }

            // second - long directions
            if (maxScore == basicSimState.score) {
                return applySelectedSimStateAsCommand(basicSimState, context, copy.get(), simulateTicks);
            } else { // one of aims
                return applySelectedSimStateAsCommand(*bestSimState, context, copy.get(), simulateTicks);
            }
        }
    }
    context.choosedCommands.clear();

    if (!food.empty()) {
        double minDist = 1e70;
        Food *nearestFood = nullptr;
        double minRad = 1e10;
        for (auto &item : me->parts) {
            if (item.getRadius() < minRad) {
                minRad = item.getRadius();
            }
        }
        for (auto &it : food) {
            if (Utils::bannedFood(it, minRad)) {
                continue;
            }
            for (auto &item : me->parts) {
                double dist = item.distanceTo(it) - item.getRadius();
                if (dist < minDist) {
                    minDist = dist;
                    nearestFood = &it;
                }
            }
        }
        if (nearestFood != nullptr) {
            return Command(nearestFood->getX(), nearestFood->getY(), false, false);
        }
    }

    if (!context.dummy || isNear(context)) {
        double x = static_cast <double> (rand() * 1. / RAND_MAX) * Constants::GAME_WIDTH();
        double y = static_cast <double> (rand() * 1. / RAND_MAX) * Constants::GAME_HEIGHT();
        while (Utils::distSqr(x, y, midX, midY) < Constants::GAME_WIDTH() / 2.1) {
            x = static_cast <double> (rand() * 1. / RAND_MAX) * Constants::GAME_WIDTH();
            y = static_cast <double> (rand() * 1. / RAND_MAX) * Constants::GAME_HEIGHT();
        }
        context.dummy = true;
        return Command(x, y, "No food");
    } else {
        return Command(context.getPrevDestX(), context.getPrevDestY(), "No food");
    }
}