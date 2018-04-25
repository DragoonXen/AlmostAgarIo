//
// Created by dragoon on 4/1/18.
//

#ifdef REWIND_VIEWER

#include "../../rewind/RewindClient.h"

#endif

#include <set>
#include "Simulator.h"
#include "../../SimpleLog.h"
#include "../../Utils.h"
#include "FoodDetection.h"

PlayerPart *getNearestPredator(const Food &food, std::list<PlayerPart *> &fragments) {
    PlayerPart *nearest_predator = nullptr;
    double deeper_dist = 0.;
    for (auto &predator : fragments) {
        double qdist = predator->checkEatingDist(food);
        if (qdist > deeper_dist) {
            deeper_dist = qdist;
            nearest_predator = predator;
        }
    }
    return nearest_predator;
};


PlayerPart *getNearestPredator(const GameObject &food, std::list<PlayerPart *> &fragments) {
    PlayerPart *nearest_predator = nullptr;
    double deeper_dist = 0.;
    for (auto &predator : fragments) {
        double qdist = predator->checkEatingDist(food);
        if (qdist > deeper_dist) {
            deeper_dist = qdist;
            nearest_predator = predator;
        }
    }
    return nearest_predator;
};

Virus *getNearestVirus(const Ejection &food, std::list<Virus> &fragments) {
    Virus *nearest_predator = nullptr;
    double deeper_dist = 0.;
    for (auto &predator : fragments) {
        double qdist = predator.checkEatingDist(food);
        if (qdist > deeper_dist) {
            deeper_dist = qdist;
            nearest_predator = &predator;
        }
    }
    return nearest_predator;
};

inline void removePlayerPart(const PlayerPart &part, std::list<PlayerPart> &parts) {
    for (auto it = parts.begin(); it != parts.end(); ++it) {
        if (it->getId() == part.getId()) {
            parts.erase(it);
            return;
        }
    }
}

inline double eat_food(Context &context) {
    auto &fragments = context.allPlayersParts;
    double score = 0.;
    for (auto fit = context.food.begin(); fit != context.food.end();) {
        if (auto *eater = getNearestPredator(*fit, fragments)) {
            eater->eat(*fit);
            if (eater->getPlayerId() == context.me->id) {
                score += 1.f;
            } else {
                score -= 1.f * Constants::ENEMY_EAT_NEGATION_KOEFF;
            }
            fit = context.food.erase(fit);
        } else {
            fit++;
        }
    }
    return score;
}

inline double eat_ejections(Context &context) {
    double score = 0.;
    auto &fragments = context.allPlayersParts;
    for (auto eit = context.ejections.begin(); eit != context.ejections.end();) {
        auto eject = *eit;
        if (Virus *virusEater = getNearestVirus(eject, context.viruses)) {
            virusEater->eat(eject);
        } else if (PlayerPart *eater = getNearestPredator(eject, fragments)) {
            eater->eat(eject);
            if (eater->getPlayerId() == context.me->id){
                score += 1.;
            }
        } else {
            eit++;
            continue;
        }
        eit = context.ejections.erase(eit);
    }
    return score;
}

inline double eat_players(Context &context, int simsRemained) {
    double score = 0.;
    bool eated = false;
    for (Player &player : context.players) {
        for (auto pit = player.parts.begin(); pit != player.parts.end();) {
            PlayerPart &food = *pit;
            PlayerPart *nearest_predator = nullptr;
            double deeper_dist = 0.;
            for (Player &predatorPlayer : context.players) {
                if (&predatorPlayer == &player) {
                    continue;
                }
                for (PlayerPart &predator : predatorPlayer.parts) {
                    if (predator.getMass() <= food.getMass() * Constants::MASS_EAT_FACTOR) { // can't eat
                        continue;
                    }

                    double sqrDistance = predator.sqrDist(food);
//                    double radiusSum = predator.getRadius() + food.getRadius();
//                    radiusSum *= radiusSum;
//                    if (radiusSum < sqrDistance) {
                    double coverDist = predator.getRadius() - food.getRadius() * Constants::EATING_RAD;

                    if (coverDist * coverDist >= sqrDistance) {
                        double qdist = predator.getRadius() - sqrt(sqrDistance);
                        if (qdist > deeper_dist) {
                            deeper_dist = qdist;
                            nearest_predator = &predator;
                        }
                        // food.updateDanger(1.); no need in danger, cause it'll be eated
                    } else {
                        if (&player == context.me || &predatorPlayer == context.me) {
                            // distance == radiusSum -> 0
                            // distance == coverDist -> 1
                            // TODO: play with this expression
                            food.updateDanger(coverDist * coverDist / sqrDistance);// * Constants::INDEX_MULT[simsRemained]);
                        }
//                        }
                    }
                }

            }
            if (nearest_predator != nullptr) {
                nearest_predator->eat(food);
                if (food.getPlayerId() == context.me->id) {
                    bool last = (context.me->parts.size() == 1);
                    score -= last ? Constants::ENEMY_DESTROY_ME : Constants::ENEMY_EAT_ME_SCORE;
                } else {
                    if (nearest_predator->getPlayerId() == context.me->id) {
                        bool last = (context.players[food.getPlayerId()].parts.size() == 1);
                        score += last ? Constants::ME_DESTROY_ENEMY : Constants::ME_EAT_ENEMY; // TODO: add check if he is really last
//                } else {
//                    score -= Constants::ENEMY_EAT_ANY_SCORE;
                    }
                }
                pit = player.parts.erase(pit);
                eated = true;
            } else {
                pit++;
            }
        }
    }
    if (eated) {
        context.updatePlayerParts();
    }
    return score;
}

inline double eat_all(Context &context, int simsRemained) {
    double score = eat_food(context);
    score += eat_ejections(context);
    return score + eat_players(context, simsRemained);
}

inline bool fuse_players(std::list<PlayerPart> &parts) {
    std::list<PlayerPart *> fragments;
    for (auto &part : parts) {
        if (part.getTtf() == 0) {
            fragments.push_back(&part);
        }
    }
    if (fragments.size() < 2) {
        return false;
    }
    // приведём в предсказуемый порядок
    fragments.sort([](const PlayerPart *a, const PlayerPart *b) -> bool {
        if (a->getMass() == b->getMass()) {
            return a->getId() < b->getId();
        } else {
            return a->getMass() > b->getMass();
        }
    });
    bool new_fusion_check = true; // проверим всех. Если слияние произошло - перепроверим ещё разок, чтобы все могли слиться в один тик
    std::set<int> fusedPlayers;
    bool fused = false;
    while (new_fusion_check) {
        new_fusion_check = false;
        for (auto it = fragments.begin(); it != fragments.end(); ++it) {
            auto &player = *it;
            for (auto it2 = std::next(it); it2 != fragments.end();) {
                auto &frag = *it2;
                if (player->fuse(*frag)) {
                    fusedPlayers.insert(frag->getId());
                    new_fusion_check = true;
                    it2 = fragments.erase(it2);
                } else {
                    ++it2;
                }
            }
        }
        fused |= new_fusion_check;
        if (new_fusion_check) {
            for (auto &fragment : fragments) {
                fragment->updateByMass();
            }
        }
    }

    for (auto it = parts.begin(); it != parts.end();) {
        if (fusedPlayers.find(it->getId()) != fusedPlayers.end()) {
            it = parts.erase(it);
        } else {
            ++it;
        }
    }
    return fused;
}

inline void fuse_players(Context &context) {
    std::vector<Player *> players;
    players.push_back(context.me);
    bool fused = false;
    for (auto &item : context.players) {
        if (!item.parts.empty()) {
            fused |= fuse_players(item.parts);
        }
    }
    if (fused) {
        context.updatePlayerParts();
    }
}

inline double burstPlayer(Context &context, PlayerPart &playerPart, Virus &virus) {
    double dist = playerPart.distanceTo(virus);
    double dy = playerPart.getY() - virus.getY();
    double angle = 0.;

    if (dist > 0) {
        angle = std::asin(dy / dist);
        if (playerPart.getX() < virus.getX()) {
            angle = M_PI - angle;
        }
    }
//    double max_speed = Constants::SPEED_FACTOR() / std::sqrt(playerPart.getMass());
//    if (speed < max_speed) {
//        speed = max_speed;
//    }
    playerPart.addMass(Constants::BURST_BONUS);

    int newFragsCnt = int(playerPart.getMass() / Constants::MIN_BURST_MASS) - 1;
    auto &parts = context.players[playerPart.getPlayerId()].parts;
    newFragsCnt = std::min(newFragsCnt, Constants::MAX_FRAGS_CNT() - (int) parts.size());

    double newMass = playerPart.getMass() / (newFragsCnt + 1);
    double newRadius = playerPart.mass2radius(newMass);

    int maxId = 0;
    for (auto &part : parts) {
        maxId = std::max(maxId, part.getId());
    }

    for (int i = 0; i != newFragsCnt; ++i) {
        //int id, int playerId, double x, double y, double sx, double sy, double m, double r, int updateTick, int ttf
        PlayerPart &newFragment = parts.emplace_back(maxId + i + 1,
                                                     playerPart.getPlayerId(),
                                                     playerPart.getX(),
                                                     playerPart.getY(),
                                                     0.,
                                                     0.,
                                                     newMass,
                                                     newRadius,
                                                     context.getTickIndex(),
                                                     Constants::TICKS_TIL_FUSION());
        newFragment.updateStartMass();
        context.allPlayersParts.push_back(&newFragment);
        double burstAngle = angle - Constants::BURST_ANGLE_SPECTRUM / 2 + i * Constants::BURST_ANGLE_SPECTRUM / newFragsCnt;
        newFragment.setImpulse(Constants::BURST_START_SPEED, burstAngle);
    }
    playerPart.burstUpdate(maxId + newFragsCnt + 1, newMass, newRadius);

    playerPart.setImpulse(Constants::BURST_START_SPEED, angle + Constants::BURST_ANGLE_SPECTRUM / 2);

    if (playerPart.getPlayerId() == context.me->id) {
        return Constants::SCORE_FOR_BURST;
    } else {
        return Constants::SCORE_FOR_ENEMY_BURST;
    }
}

PlayerPart *getNearestPlayer(const Virus &virus, const Context &context) {
    PlayerPart *nearestVictim = nullptr;
    double nearestDist = INFINITY;

    auto &parts = context.me->parts;
    size_t yet_cnt = parts.size();
    for (auto &playerPart : parts) {
        if (playerPart.canBurst(yet_cnt)) {
            double qdist = virus.canHurt(playerPart);
            if (qdist < nearestDist) {
                nearestDist = qdist;
                nearestVictim = &playerPart;
            }
        }
    }
    return nearestVictim;
};

inline double burstOnViruses(Context &context) {
    double scores = 0.;
    for (auto vit = context.viruses.begin(); vit != context.viruses.end();) {
        if (PlayerPart *player = getNearestPlayer(*vit, context)) {
            scores += burstPlayer(context, *player, *vit);
            vit = context.viruses.erase(vit);
        } else {
            vit++;
        }
    }
    return scores;
}

inline void splitNow(PlayerPart *part, Context &context, int max_fId) {
    double newMass = part->getMass() / 2;
    double newRadius = PlayerPart::mass2radius(newMass);
    auto &newPlayer = context.players[part->getPlayerId()].parts.emplace_back(max_fId,
                                                                              part->getPlayerId(),
                                                                              part->getX(),
                                                                              part->getY(),
                                                                              0.,
                                                                              0.,
                                                                              newMass,
                                                                              newRadius,
                                                                              context.getTickIndex(), Constants::TICKS_TIL_FUSION());
    newPlayer.updateStartMass();
    double angle = atan2(part->getDy(), part->getDx());
    newPlayer.setImpulse(Constants::SPLIT_START_SPEED, angle);
    part->burstUpdate(max_fId + 1, newMass, newRadius);
    context.allPlayersParts.push_back(&newPlayer);
}

inline void processSplit(Context &context, const SimCommand &command) {
    if (!command.split) {
        return;
    }

    std::vector<PlayerPart *> parts;
    parts.reserve(context.me->parts.size());
    for (auto &playerPart : context.me->parts) {
        parts.push_back(&playerPart);
    }
    std::sort(parts.begin(), parts.end(), [](const PlayerPart *lhs, const PlayerPart *rhs) {
        return
                std::make_tuple(lhs->getMass(), lhs->getId()) >
                std::make_tuple(rhs->getMass(), rhs->getId());
    });

    int partsCount = (int) context.me->parts.size();
    int maxId = 0;
    for (auto &part : parts) {
        maxId = std::max(maxId, part->getId());
    }
    for (PlayerPart *frag : parts) {
        if (frag->canSplit(partsCount)) {
            ++maxId;
            splitNow(frag, context, maxId);
            ++maxId;
            ++partsCount;
        }
    }
}

inline double simulateTick(Context &context, const SimCommand &command, int remainedTicks) {
    ++context.tickIndex;

    auto &fragments = context.me->parts;
    for (auto &player : fragments) {
        player.updateVector(command.x, command.y);
        player.updateStartMass();
    }
    for (auto &player : context.players) {
        if (&player == context.me) {
            continue;
        }
        if (std::isnan(player.aimX)) {
            for (auto &playerPart : player.parts) {
                playerPart.updateStartMass();
            }
        } else {
            for (auto &playerPart : player.parts) {
                playerPart.updateVector(player.aimX, player.aimY);
                playerPart.updateStartMass();
            }
        }
    }

    for (auto &player : context.players) {
        auto &parts = player.parts;
        for (auto it = parts.begin(); it != parts.end(); ++it) {
            for (auto it2 = std::next(it); it2 != parts.end(); ++it2) {
                it->collisionCalc(*it2);
            }
        }
    }
    for (auto &player : context.allPlayersParts) {
        player->move();
    }
    processSplit(context, command); // only self split
    if (context.tickIndex % Constants::SHRINK_EVERY_TICK == 0) {
        for (auto &player : context.allPlayersParts) {
            player->shrink();
        }
    }

    double score = eat_all(context, remainedTicks);
    fuse_players(context);

    score += burstOnViruses(context);

    for (auto &player : context.allPlayersParts) {
        player->updateByMass();
        player->fixSpeedAfterCollision();
    }
    return score;
}

void Simulator::externalSimulateTick(Context &context, double aimX, double aimY) {
    context.updatePlayerParts();
    for (auto &player : context.allPlayersParts) {
        player->updateMaxSpeed();
    }
    simulateTick(context, SimCommand(aimX, aimY), 0);
}

inline bool updateXY(int userId, int fragmentId, const Context &context, double &x, double &y) {
    for (auto &part : context.players[userId].parts) {
        if (part.getId() == fragmentId) {
            x = part.getX();
            y = part.getY();
            return true;
        }
    }
    return false;
}

double Simulator::simulate(Context context, const std::vector<std::pair<SimCommand, int>> &commandList, int totalTicks, bool cleanDetect) {
    if (cleanDetect) {
        for (auto &player : context.players) {
            player.aimX = NAN;
        }
    }
    context.updatePlayerParts();
    for (auto &player : context.allPlayersParts) {
        player->updateMaxSpeed();
    }

    auto iterator = commandList.begin();
    double scores = 0.;
    int cnt = iterator->second;
    double ppMax = 0.;
    const SimCommand *command = &iterator->first;
    double aimX(command->x), aimY(command->y);
    bool updateAim = (command->userId != -1);

    while (totalTicks--) {
        if (cnt == 0) {
            ++iterator;
            if (iterator == commandList.end()) {
                LOG_DEBUG("command list is empty!");
                return scores;
            }
            cnt = iterator->second;
        }
        --cnt;
        scores *= Constants::PREV_TICK_SCORE_MULT;
        if (updateAim) {
            updateAim = updateXY(command->userId, command->fragmentId, context, aimX, aimY);
        }
        scores += simulateTick(context, iterator->first, totalTicks);
        for (auto &playerPart : context.me->parts) {
            ppMax = std::max(FoodDetection::getFoodPP(playerPart.getX(), playerPart.getY()), ppMax); // * Constants::INDEX_MULT[totalTicks]
        }
    }
    for (auto &playerPart : context.allPlayersParts) {
        if (playerPart->getPlayerId() == context.me->id) {
            scores -= context.me->parts.size() > 1 ? Constants::MY_DANGER_COEFF * playerPart->getDanger() : Constants::MY_DANGER_DESTROY *
                                                                                                            playerPart->getDanger();
        } else {
            scores += Constants::ENEMY_DANGER_COEFF * playerPart->getDanger();
        }
    }
    if (context.me->parts.empty()) {
        return scores;
    }

    scores += ppMax * Constants::FOOD_PP_SCORE_BONUS;
//    FIELD_DANGER
    for (auto &part : context.me->parts) {
        scores -= Constants::FIELD_DANGER[(int) part.getX()][(int) part.getY()];
    }

//    if (scores == 0.) {
//        double sumSpeed = 0.;
//        for (auto &player : context.me->parts) {
//            sumSpeed += Utils::len(player.getDx(), player.getDy());
//        }
//        sumSpeed /= context.me->parts.size();
//        return sumSpeed * Constants::SPEED_SCORE_BONUS;
//    } else {
    return scores;
//    }
}

std::pair<double, SimulateType>
Simulator::simulateProxy(const ExtContext &context,
                         const std::vector<std::pair<SimCommand, int>> &commandList,
                         int totalTicks,
                         Context const *const maxTtfContext, double currentMax) {
    bool repeat = false;
    for (auto &player : context.players) {
        repeat |= (!std::isnan(player.aimX));
    }
    SimulateType simType = SimulateType::MIN_TTF_NODETECT;
    double score = simulate(context, commandList, totalTicks, true);
    if (currentMax > score) {
        return std::make_pair(score, simType);
    }
    if (repeat) {
        double newScore = simulate(context, commandList, totalTicks, false);
        if (newScore < score) {
            simType = SimulateType::MIN_TTF_DETECT;
            score = newScore;
            if (currentMax > score) {
                return std::make_pair(score, simType);
            }
        }
    }
    if (maxTtfContext == nullptr) {
        return std::make_pair(score, simType);
    }
    double maxTtfScore = simulate(*maxTtfContext, commandList, totalTicks, true);
    if (maxTtfScore < score) {
        score = maxTtfScore;
        simType = SimulateType::MAX_TTF_NODETECT;
        if (currentMax > score) {
            return std::make_pair(score, simType);
        }
    }
    if (repeat) {
        maxTtfScore = simulate(*maxTtfContext, commandList, totalTicks, false);
        if (maxTtfScore < score) {
            score = maxTtfScore;
            simType = SimulateType::MAX_TTF_DETECT;
        }
    }
    return std::make_pair(score, simType);
}

#ifdef REWIND_VIEWER

double Simulator::debugSimulate(Context context, const std::vector<std::pair<SimCommand, int>> &commandList, int totalTicks, bool cleanDetect) {
    if (cleanDetect) {
        for (auto &player : context.players) {
            player.aimX = NAN;
        }
    }
    context.updatePlayerParts();
    auto iterator = commandList.begin();
    double scores = 0.;
    for (auto &player : context.allPlayersParts) {
        player->updateMaxSpeed();
    }
    int cnt = iterator->second;
    double ppMax = 0.;
    const SimCommand *command = &iterator->first;
    double aimX(command->x), aimY(command->y);
    bool updateAim = (command->userId != -1);

    while (totalTicks--) {
        if (cnt == 0) {
            ++iterator;
            if (iterator == commandList.end()) {
                LOG_DEBUG("command list is empty!");
                return scores;
            }
            cnt = iterator->second;
        }
        --cnt;
        scores *= Constants::PREV_TICK_SCORE_MULT;
        if (updateAim) {
            updateAim = updateXY(command->userId, command->fragmentId, context, aimX, aimY);
        }
        scores += simulateTick(context, iterator->first, totalTicks);
        for (auto &playerPart : context.me->parts) {
            ppMax = std::max(FoodDetection::getFoodPP(playerPart.getX(), playerPart.getY()), ppMax); //  * Constants::INDEX_MULT[totalTicks]
        }

        uint32_t playerColor[] = {0xFFFF00, 0xFF00FF, RewindClient::COLOR_BLUE, RewindClient::COLOR_GREEN, 0x00FFFF};
        RewindClient &client = RewindClient::instance();
        for (auto &player : context.players) {
            for (auto &playerPart : player.parts) {
                client.circle(playerPart.getX(), playerPart.getY(), playerPart.getRadius(), playerColor[playerPart.getPlayerId()] | 0x11000000, 2);
            }
        }
    }
    for (auto &playerPart : context.allPlayersParts) {
        if (playerPart->getPlayerId() == context.me->id) {
            scores -= context.me->parts.size() > 1 ? Constants::MY_DANGER_COEFF * playerPart->getDanger() : Constants::MY_DANGER_DESTROY *
                                                                                                            playerPart->getDanger();
        } else {
            scores += Constants::ENEMY_DANGER_COEFF * playerPart->getDanger();
        }
    }
    if (context.me->parts.empty()) {
        return scores;
    }

    scores += ppMax * Constants::FOOD_PP_SCORE_BONUS;
//    FIELD_DANGER
    for (auto &part : context.me->parts) {
        scores -= Constants::FIELD_DANGER[(int) part.getX()][(int) part.getY()];
    }

//    if (scores == 0.) {
//        double sumSpeed = 0.;
//        for (auto &player : context.me->parts) {
//            sumSpeed += Utils::len(player.getDx(), player.getDy());
//        }
//        sumSpeed /= context.me->parts.size();
//        return sumSpeed * Constants::SPEED_SCORE_BONUS;
//    } else {
    return scores;
//    }
}

double Simulator::debugSimulateProxy(const ExtContext &context,
                                     const std::vector<std::pair<SimCommand, int>> &commandList,
                                     int totalTicks,
                                     Context const *const maxTtfContext,
                                     SimulateType simulateType) {

    bool cleanDetect = (simulateType == MAX_TTF_NODETECT || simulateType == MIN_TTF_NODETECT);
    debugSimulate((simulateType == MAX_TTF_NODETECT || simulateType == MAX_TTF_DETECT) ? *maxTtfContext : context, commandList, totalTicks, cleanDetect);
    return 0;
}

#endif