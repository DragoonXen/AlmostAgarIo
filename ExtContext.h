//
// Created by dragoon on 4/7/18.
//

#ifndef ALMOSTAGARIO_EXTCONTEXT_H
#define ALMOSTAGARIO_EXTCONTEXT_H

#ifdef REWIND_VIEWER
#include "../rewind/RewindClient.h"
#endif

#include <set>
#include "Context.h"
#include "models/simulation/ViewCircle.h"
#include "SimpleLog.h"
#include "models/simulation/EnemyDirection.h"
#include "models/simulation/EnemyDirectionMem.h"
#include "models/simulation/SimCommand.h"

#include "models/simulation/KnownFood.h"

class ExtContext : public Context {
public:
    static constexpr double REMEMBER_NEGATION = 1000.;
    static constexpr double EPS = 1.;


    ExtContext(int tickIndex);

    std::list<int> simTicks;
    std::list<int> turnTime;
    int totalTicks;
    int totalTurnTime;
    int lastSimCount;
    long totalLength;
    int tlPrevention;

    bool disableAddPoints;

    std::map<std::pair<int, int>, int> ttf;
    std::set<std::pair<int, int>> suspicious;
    std::set<std::pair<int, int>> nextStepSuspicious;

    std::vector<std::pair<SimCommand, int>> choosedCommands;

    std::vector<ViewCircle> vision;
    std::list<std::pair<int, int>> prevTickFood;

    double prob[Constants::EAT_FIELD_COUNT][Constants::EAT_FIELD_COUNT];
    int lastSeenTick[Constants::EAT_FIELD_COUNT][Constants::EAT_FIELD_COUNT];
    std::vector<KnownFood> knownFood[Constants::EAT_FIELD_COUNT][Constants::EAT_FIELD_COUNT];

    std::vector<EnemyDirectionMem> directionMem[4];

    double &getFoodProbItem(double cX, double cY) {
        int x, y;
        getSquare(cX, cY, x, y);
        return prob[x][y];
    }

    inline bool foodExist(double cX, double cY) {
        return foodExist((int) (cX + 0.1), (int) (cY + 0.1));
    }

    inline bool foodExist(int cX, int cY) {
        int x, y;
        getSquare(cX, cY, x, y);
        auto &ptr = knownFood[x][y];
        for (auto &knownFood : ptr) {
            if (knownFood.x == cX && knownFood.y == cY) {
                return true;
            }
        }
        return false;
    }

    static inline void getSquare(double cX, double cY, int &outX, int &outY) {
        outX = (int) (cX / Constants::COORDS_IN_ONE_FOOD_FIELD);
        outY = (int) (cY / Constants::COORDS_IN_ONE_FOOD_FIELD);
    }

    inline void addFood(int cX, int cY) {
        int x, y;
        getSquare(cX, cY, x, y);
        knownFood[x][y].emplace_back(cX, cY);
    }

    void rememberSeen(int i, int j) {
        if (prob[i][j] >= 0) {
            prob[i][j] -= REMEMBER_NEGATION;
        }
    }

    void updateTtfTick();

    void rememberLastDirections(const Player &player) {
        auto &parts = player.parts;
        auto &directionMem = this->directionMem[player.id];
        directionMem.clear();
        for (auto &part : parts) {
            if (part.getDx() == 0 || part.getDy() == 0 || part.isFast()) { // skip this step founded enemies
                continue;
            }

            double dx = part.getDx();
            double dy = part.getDy();
            if (part.getX() == part.getRadius() || part.getX() == Constants::GAME_WIDTH() - part.getRadius()) {
                dx = 0;
            }
            if (part.getY() == part.getRadius() || part.getY() == Constants::GAME_HEIGHT() - part.getRadius()) {
                dy = 0;
            }

            if (part.getId() == 0) {
                directionMem.emplace_back(part.getId(), part.getX(), part.getY(), dx, dy, 0., 0., part.getMass(), true);
                continue;
            }

            bool fullCover = false;
            for (auto &vc : vision) {
                double sum = Utils::dist(vc.cX, vc.cY, part.getX(), part.getY()) + part.getRadius();
                if (sum < vc.radius) {
                    fullCover = true;
                    break;
                }
            }

            double cX = 0.;
            double cY = 0.;
            for (auto &item : parts) {
                if (&item == &part || item.isFast()) {// no check on himself or splitted item
                    continue;
                }
                part.applyMyCollision(item, cX, cY);
            }
            directionMem.emplace_back(part.getId(), part.getX(), part.getY(), dx, dy, cX, cY, part.getMass(), fullCover);
        }
    }

    inline bool getIntersection(const EnemyDirection &first, const EnemyDirection &second, double &x, double &y) {
        double znam = (first.x1 - first.x2) * (second.y1 - second.y2) - (first.y1 - first.y2) * (second.x1 - second.x2);
        if (std::abs(znam) < 1e-9) {
            return false;
        }
        double leftSide = first.x1 * first.y2 - first.y1 * first.x2;
        double rightSide = second.x1 * second.y2 - second.y1 * second.x2;
        x = (leftSide * (second.x1 - second.x2) - rightSide * (first.x1 - first.x2)) / znam;
        y = (leftSide * (second.y1 - second.y2) - rightSide * (first.y1 - first.y2)) / znam;
        return true;
    };

    void calcAims(Player &player, int tickIndex) {
        bool hasSeen = false;
        for (auto &part : player.parts) {
            if (part.getUpdateTick() == tickIndex) {
                hasSeen = true;
                break;
            }
        }
        if (!hasSeen) {
            return;
        }
        player.aimX = std::numeric_limits<double>::quiet_NaN();
        auto &directionMem = this->directionMem[player.id];
        auto &parts = player.parts;
        if (directionMem.empty() || parts.empty()) {
            return;
        }

#ifdef REWIND_VIEWER
        RewindClient &client = RewindClient::instance();

        for (auto &part : parts) {
            if (part.getX() == part.getRadius() || part.getX() == Constants::GAME_WIDTH() - part.getRadius()) {
                continue;
            }
            if (part.getY() == part.getRadius() || part.getY() == Constants::GAME_HEIGHT() - part.getRadius()) {
                continue;
            }
            for (auto &dirMem : directionMem) {
                if (dirMem.fragId == part.getId()) {
                    // this part.
                    double CONST_C = Constants::INERTION_FACTOR() / dirMem.mass;
                    double MAX_SPEED = Constants::SPEED_FACTOR() / sqrt(dirMem.mass);

                    double vecX = part.getDx() - dirMem.cX + (CONST_C - 1) * dirMem.dx;
                    double vecY = part.getDy() - dirMem.cY + (CONST_C - 1) * dirMem.dy;

                    Utils::toWall(dirMem.x, dirMem.y, vecX, vecY, CONST_C * MAX_SPEED);

                    double aimX = dirMem.x + vecX;
                    double aimY = dirMem.y + vecY;
                    client.line(dirMem.x, dirMem.y, aimX, aimY, dirMem.exact ? 0x0000FF : 0x000000, 4);
                    break;
                }
            }
        }
#endif

        std::vector<EnemyDirection> direction;
        int exactDirt = 0;
        for (auto &part : parts) {
            if (part.getUpdateTick() != tickIndex) {
                continue;
            }
            if (part.getX() == part.getRadius() || part.getX() == Constants::GAME_WIDTH() - part.getRadius()) {
                continue;
            }
            if (part.getY() == part.getRadius() || part.getY() == Constants::GAME_HEIGHT() - part.getRadius()) {
                continue;
            }
            for (auto &dirMem : directionMem) {
                if (dirMem.fragId == part.getId()) {
                    // this part.
                    double CONST_C = Constants::INERTION_FACTOR() / dirMem.mass;
//                    double MAX_SPEED = Constants::SPEED_FACTOR() / sqrt(dirMem.mass);

                    double vecX = part.getDx() - dirMem.cX + (CONST_C - 1) * dirMem.dx;
                    double vecY = part.getDy() - dirMem.cY + (CONST_C - 1) * dirMem.dy;

                    direction.emplace_back(dirMem.x, dirMem.y, dirMem.x + vecX, dirMem.y + vecY, dirMem.exact);
                    exactDirt += dirMem.exact;
                }
            }
        }

        if (exactDirt == 0) {
            if (direction.size() < 3) { // too not exact data
                return;
            }
            std::vector<std::pair<double, double>> intersections;
            for (auto it = direction.begin(); it != direction.end(); ++it) {
                for (auto it2 = std::next(it); it2 != direction.end(); ++it2) {
                    double x, y;
                    if (getIntersection(*it, *it2, x, y)) {
                        intersections.emplace_back(x, y);
                    }
                }
            }

            int maxCnt = 0;
            std::pair<double, double> *bestPtr = nullptr;
            for (auto it = intersections.begin(); it != intersections.end(); ++it) {
                int cnt = 0;
                for (auto it2 = intersections.begin(); it2 != intersections.end(); ++it2) {
                    if (it == it2) {
                        continue;
                    }
                    cnt += (std::abs(it->first - it2->first) + std::abs(it->second - it2->second)) < EPS;
                }
                if (cnt > maxCnt) {
                    maxCnt = cnt;
                    bestPtr = &(*it);
                }
            }
            if (maxCnt > 1) { // at least three lines intersect in one point
                player.aimX = bestPtr->first;
                player.aimY = bestPtr->second;
            }
        } else if (exactDirt == 1) {
            EnemyDirection *exactPtr(nullptr);
            for (auto &item :direction) {
                if (item.exact) {
                    exactPtr = &item;
                    break;
                }
            }

            std::vector<std::pair<double, double>> intersections;
            for (auto &it2 : direction) {
                double x, y;
                if (getIntersection(*exactPtr, it2, x, y)) {
                    intersections.emplace_back(x, y);
                }
            }

            int maxCnt = 0;
            std::pair<double, double> *bestPtr = nullptr;
            for (auto it = intersections.begin(); it != intersections.end(); ++it) {
                int cnt = 0;
                for (auto it2 = intersections.begin(); it2 != intersections.end(); ++it2) {
                    if (it == it2) {
                        continue;
                    }
                    cnt += (std::abs(it->first - it2->first) + std::abs(it->second - it2->second)) < EPS;
                }
                if (cnt > maxCnt) {
                    maxCnt = cnt;
                    bestPtr = &(*it);
                }
            }
            if (maxCnt > 0) { // at least three lines intersect in one point
                player.aimX = bestPtr->first;
                player.aimY = bestPtr->second;
            } else {
                double aimDx = exactPtr->x2 - exactPtr->x1;
                double aimDy = exactPtr->y2 - exactPtr->y1;
                Utils::toWall(exactPtr->x1, exactPtr->y1, aimDx, aimDy, 1e-6);
                player.aimX = exactPtr->x1 + aimDx;
                player.aimY = exactPtr->y1 + aimDy;
            }
        } else {
            EnemyDirection *first(nullptr), *second(nullptr);
            for (auto &item :direction) {
                if (item.exact) {
                    first = second;
                    second = &item;
                    if (first != nullptr) {
                        if (getIntersection(*first, *second, player.aimX, player.aimY)) {
                            return;
                        } else {
                            LOG_ERROR("two exact lines have no intersections");
                        }
                    }
                }
            }
        }
    }
};


#endif //ALMOSTAGARIO_EXTCONTEXT_H
