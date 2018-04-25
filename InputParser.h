//
// Created by dragoon on 3/29/18.
//

#ifndef ALMOSTAGARIO_INPUTPARSER_H
#define ALMOSTAGARIO_INPUTPARSER_H

#include "../nlohmann/json.hpp"
#include "ExtContext.h"
#include "SimpleLog.h"
#include "Utils.h"
#include "models/simulation/FoodDetection.h"
#include "models/simulation/Simulator.h"

using json = nlohmann::json;

class InputParser {

    inline static bool checkPlayer(ExtContext &context, std::pair<int, int> &id, PlayerPart &playerPart, double speed, double burstSpeed) {
        double diff = burstSpeed - speed;
        if (diff >= -1e-8) {
            double cnt = diff / Constants::VISCOSITY();
            double rounded = std::round(cnt);
            if (std::abs(rounded - cnt) < 1e-8) { //close match with expected
                int passed = (int) (rounded + 0.1);
                playerPart.updateTTF(Constants::TICKS_TIL_FUSION() - passed - 1);
                playerPart.setIsFast();
                auto suspiciousIter = context.suspicious.find(id);
                if (suspiciousIter == context.suspicious.end()) { // if wasn't suspicios - put in susp map
                    context.nextStepSuspicious.insert(id);
                    LOG_DEBUG("tick % put % % to susp map, ttl %", context.getTickIndex(), id.first, id.second, Constants::TICKS_TIL_FUSION() - passed - 1)
                } else { // else fix it's ttf - we are sure
                    context.ttf[id] = Constants::TICKS_TIL_FUSION() - passed - 1;
                    LOG_DEBUG("tick % put % % to map, ttl %", context.getTickIndex(), id.first, id.second, Constants::TICKS_TIL_FUSION() - passed - 1)
                }
                return true;
            } else {
                return false;
            }
        }
        return false;
    }

    inline static void checkPlayer(ExtContext &context, PlayerPart &playerPart) {
        playerPart.updateMaxSpeed();
        auto pair = std::make_pair(playerPart.getPlayerId(), playerPart.getId());
        auto ttfIter = context.ttf.find(pair);
        if (ttfIter != context.ttf.end()) {
            playerPart.updateTTF(ttfIter->second);
            return;
        }

        double currSpeed = Utils::len(playerPart.getDx(), playerPart.getDy());
        if (currSpeed <= playerPart.getMaxSpeed()) {
            pair.second = 0;
            auto iter = context.ttf.lower_bound(pair);
            if (iter->first.first == playerPart.getPlayerId()) { // if has any ttf for that player
                int maxTtf = 0;
                while (iter != context.ttf.end() && iter->first.first == playerPart.getPlayerId() && iter->first.second < playerPart.getId()) {
                    if (iter->second < maxTtf) {
                        LOG_ERROR("oops. % % has % ttf, but there % ttf before", iter->first.first, iter->first.second, iter->second, maxTtf);
                        iter->second = maxTtf;
                    }
                    maxTtf = iter->second;
                    ++iter;
                }
                playerPart.updateTTF(maxTtf);
            }
            return;
        }

        if (!checkPlayer(context, pair, playerPart, currSpeed, Constants::BURST_START_SPEED)) {
            if (!checkPlayer(context, pair, playerPart, currSpeed, Constants::SPLIT_START_SPEED)) {
                playerPart.updateTTF(0);
            }
        }
    }

public:
    static void parse(ExtContext &context, json &input) {
        Context copiedContext = context;
        context.updateTtfTick();
        auto mine = input["Mine"];
        {
            if (context.getTickIndex() == 0) {
                std::string val = mine[0]["Id"];
                context.me = &context.players[std::stoi(val) - 1];
            }
            auto &me = context.me->parts;
//        me.clear();
            for (auto it : mine) {
                /*
                 * const std::string &id, double x, double y, double dx, double dy, double mass, double radius, int updateTick, int ttf)
                 */
                std::string id;
                int playerId, partId;
                double x, y, sx, sy, m, r;
                int ttf = 0;
                id = it["Id"];
                Utils::splitPlayerId(id, playerId, partId);
                x = it["X"];
                y = it["Y"];
                sx = it["SX"];
                sy = it["SY"];
                m = it["M"];
                r = it["R"];
                if (it.find("TTF") != it.end()) {
                    ttf = it["TTF"];
                }
                bool added = false;
                for (auto &player : me) {
                    if (player.getId() == partId) {
                        player.updateVals(x, y, sx, sy, m, r, ttf, context.getTickIndex());
                        added = true;
//                        LOG_DEBUG("fixed, speed %, angle %, dx %, dy %, x %, y %",
//                                  player.getSpeed(),
//                                  player.getAngle(),
//                                  player.getDx(),
//                                  player.getDy(),
//                                  player.getX(),
//                                  player.getY());
                        break;
                    }
                }
                if (!added) {
                    me.emplace_back(partId, playerId, x, y, sx, sy, m, r, context.getTickIndex(), ttf);
                }
            }
            for (auto it = me.begin(); it != me.end();) {
                if (it->getUpdateTick() != context.getTickIndex()) {
                    it = me.erase(it);
                } else {
                    ++it;
                }
            }
        }
        auto &me = context.me;
        if (me->parts.empty()) {
            LOG_ERROR("WAT? Died");
            return;
        }
        auto objects = input["Objects"];
        auto &players = context.players;
        auto &food = context.food;
        auto &viruses = context.viruses;
        auto &ejections = context.ejections;

//        players.clear();
        std::vector<PlayerPart> newPlayers;
        food.clear();
        viruses.clear();
        ejections.clear();

        bool added;
        std::string id;
        double x, y, m, r;
        int playerId, partId;
        for (auto &it : objects) {
            switch (it["T"].get<std::string>()[0]) {
                case 'V': //virus
//              Virus(const std::string &id, double x, double y, double mass, int updateTick);
                    id = it["Id"];
                    x = it["X"];
                    y = it["Y"];
                    m = it["M"];
                    viruses.emplace_back(std::stoi(id), x, y, m, context.getTickIndex());
                    break;
                case 'F': //food
//              Food(const std::string &id, double x, double y, int updateTick);
                    x = it["X"];
                    y = it["Y"];
                    food.emplace_back(x, y);
                    break;
                case 'E': //ejection
//              Ejection(const std::string &id, double x, double y, int updateTick);
                    playerId = it["pId"];
                    id = it["Id"];
                    x = it["X"];
                    y = it["Y"];
                    ejections.emplace_back(std::stoi(id), x, y, playerId, context.getTickIndex());
                    break;
                case 'P': //player
                    //PlayerPart(const std::string &id, double x, double y, double dx, double dy, double mass, double radius, int updateTick, int ttf);
                    id = it["Id"];
                    x = it["X"];
                    y = it["Y"];
//                dx = it["SX"];
//                dy = it["SY"];
                    m = it["M"];
                    r = it["R"];

                    added = false;
                    Utils::splitPlayerId(id, playerId, partId);
                    for (auto &player : players[playerId].parts) {
                        if (player.getId() == partId) {
                            player.updateVals(x, y, m, r, context.getTickIndex());
                            checkPlayer(context, player);
                            added = true;
//                            LOG_DEBUG("fixed, speed %, angle %, dx %, dy %, x %, y %",
//                                      player.getSpeed(),
//                                      player.getAngle(),
//                                      player.getDx(),
//                                      player.getDy(),
//                                      player.getX(),
//                                      player.getY());
                            break;
                        }
                    }
                    if (!added) {
//                        LOG_DEBUG("added");
                        auto &player = players[playerId].parts.emplace_back(partId, playerId, x, y, 0.f, 0.f, m, r, context.getTickIndex(), 0);
                        checkPlayer(context, player);
                    }

//                    players.emplace_back(id, x, y, 0.f, 0.f, mass, radius, context.getTickIndex(), 0);
                    break;
                default:
                    LOG_ERROR("WAT? UNKNOWN OBJECT TYPE");
                    break;
            }
        }

//        for (auto &iter : players) {
//            auto &vector = iter.parts;
//            for (auto it = vector.begin(); it != vector.end();) {
//                if (it->getUpdateTick() != context.getTickIndex()) {
//                    it = vector.erase(it);
//                } else {
//                    ++it;
//                }
//            }
//        }

        for (auto &player : context.players) {
            if (&player == context.me) {
                continue;
            }
            context.calcAims(player, context.getTickIndex());
#ifdef REWIND_VIEWER
            if (!std::isnan(player.aimX)) {
                uint32_t playerColor[] = {0xFFFF00, 0xFF00FF, RewindClient::COLOR_BLUE, RewindClient::COLOR_GREEN, 0x00FFFF};
                RewindClient &client = RewindClient::instance();
                client.circle(player.aimX, player.aimY, 2., playerColor[player.id], 4);
            }
#endif
        }

        if (context.getTickIndex() == 0) { // food init
            FoodDetection::init(context);
        } else if ((context.getTickIndex() + Constants::ADD_FOOD_DELAY + 1) % Constants::ADD_FOOD_DELAY == 0) { // pre spawn tick
            FoodDetection::updateProbs(context);
            FoodDetection::initPreFoodSpawnTick(context);
        } else if (context.getTickIndex() % Constants::ADD_FOOD_DELAY == 0) { // spawn tick 2320 for test
            FoodDetection::initFoodSpawnTick(context);
            FoodDetection::updateProbs(context);
        } else {
            FoodDetection::updateProbs(context);
        }
        LOG_DEBUG("total food count %", KnownFood::totalCnt);

        for (auto &player : context.players) {
            if (&player == context.me) {
                continue;
            }
            context.rememberLastDirections(player);
        }

        for (int i = 0; i != 4; ++i) {
            copiedContext.players[i].aimX = context.players[i].aimX;
            copiedContext.players[i].aimY = context.players[i].aimY;
        }
        Simulator::externalSimulateTick(copiedContext, context.getPrevDestX(), context.getPrevDestY()); // single tick simulation. split is not important
        for (auto &iter : players) {
            auto &vector = iter.parts;
            for (auto it = vector.begin(); it != vector.end();) {
                if (it->getUpdateTick() + Constants::VISION_REMEMBER < context.getTickIndex() ||
                    (iter.id == context.me->id && it->getUpdateTick() != context.getTickIndex())) { // no sim more
                    it = vector.erase(it);
                    continue;
                }
                if (it->getUpdateTick() == context.getTickIndex()) {
                    ++it;
                    continue;
                }
                PlayerPart *sim = nullptr;
                for (auto &simPart : copiedContext.players[it->getPlayerId()].parts) {
                    if (simPart.getId() == it->getId()) {
                        sim = &simPart;
                        break;
                    }
                }
                if (sim == nullptr) {
                    it = vector.erase(it);
                    continue;
                }
                //can see
                bool canSee = false;
                for (auto &vc : context.vision) {
                    if (vc.canSee(*sim)) {
                        canSee = true;
                        break;
                    }
                }
                if (canSee) {
                    it = vector.erase(it);
                } else {
                    it->updateNextTick(*sim);
                    ++it;
                }
            }
        }
    }
};


#endif //ALMOSTAGARIO_INPUTPARSER_H
