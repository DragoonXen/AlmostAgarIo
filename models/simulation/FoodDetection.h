//
// Created by dragoon on 4/10/18.
//

#ifndef ALMOSTAGARIO_FOODDETECTION_H
#define ALMOSTAGARIO_FOODDETECTION_H


#include "../../Constants.h"
#include "../../ExtContext.h"
#include "ViewCircle.h"
#include "../../Utils.h"
#include "KnownFood.h"
#include "../../SimpleLog.h"

class FoodDetection {

public:

    static double foodField[Constants::EAT_FIELD_COUNT][Constants::EAT_FIELD_COUNT];
    static double backupField[Constants::EAT_FIELD_COUNT][Constants::EAT_FIELD_COUNT];
    static bool seenThisTick[Constants::EAT_FIELD_COUNT][Constants::EAT_FIELD_COUNT];

    inline static double &getFoodPP(double cX, double cY) {
        return foodField[(int) (cX / Constants::COORDS_IN_ONE_FOOD_FIELD)][(int) (cY / Constants::COORDS_IN_ONE_FOOD_FIELD)];
    }

    static constexpr double diff[3][3] = {{0.0689655172, 0.1034482759, 0.0689655172},
                                          {0.1034482759, 0.3103448276, 0.1034482759},
                                          {0.0689655172, 0.1034482759, 0.0689655172}};

    inline static void init(ExtContext &context) {
        std::vector<std::pair<int, int>> initialSets;
        for (auto &food : context.food) {
            int x = (int) (food.getX() + 0.1);
            int y = (int) (food.getY() + 0.1);
            if (x > Constants::HALF_FIELD) {
                x = Constants::FIELD_SIZE - x;
            }
            if (y > Constants::HALF_FIELD) {
                y = Constants::FIELD_SIZE - y;
            }
            initialSets.emplace_back(x, y);
        }
        std::sort(initialSets.begin(), initialSets.end());
        initialSets.erase(std::unique(initialSets.begin(), initialSets.end()), initialSets.end());
        int elapsedSets = (int) (Constants::START_FOOD_SETS - initialSets.size());
        auto &playerPart = context.me->parts.front();
        ViewCircle vc(playerPart, 1);
        int seenCount[4] = {0, 0, 0, 0};
        for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
            for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                context.prob[i][j] = 0.;
            }
        }
        for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
            for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                context.lastSeenTick[i][j] = -1;
            }
        }

        for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
            int idx = ((i >= Constants::EAT_FIELD_COUNT / 2) ? 2 : 0);
            for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                idx |= ((j >= Constants::EAT_FIELD_COUNT / 2) ? 1 : 0);
                if (vc.canSee(i, j)) {
                    ++seenCount[idx];
                    context.prob[i][j] = -1;
                    context.prob[i][Constants::EAT_FIELD_COUNT - j - 1] = -1;
                    context.prob[Constants::EAT_FIELD_COUNT - i - 1][j] = -1;
                    context.prob[Constants::EAT_FIELD_COUNT - i - 1][Constants::EAT_FIELD_COUNT - j - 1] = -1;
                    context.lastSeenTick[i][j] = 0;
                    context.lastSeenTick[i][Constants::EAT_FIELD_COUNT - j - 1] = 0;
                    context.lastSeenTick[Constants::EAT_FIELD_COUNT - i - 1][j] = 0;
                    context.lastSeenTick[Constants::EAT_FIELD_COUNT - i - 1][Constants::EAT_FIELD_COUNT - j - 1] = 0;
                }
            }
        }
        int maxSeen = 0;
        for (int i = 0; i != 4; ++i) {
            maxSeen = std::max(seenCount[i], maxSeen);
        }
        int totalNotSeenClear = Constants::TOTAL_ONE_QUARTER_CELLS - maxSeen;
        double prob = ((double) elapsedSets) / totalNotSeenClear;
        for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
            for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                if (context.prob[i][j] == -1) {
                    context.prob[i][j] = 0;
                } else {
                    context.prob[i][j] = prob;
                }
            }
        }

        for (auto &pair : initialSets) {
            context.getFoodProbItem(pair.first, pair.second) += 1.;
            context.getFoodProbItem(Constants::FIELD_SIZE - pair.first, pair.second) += 1.;
            context.getFoodProbItem(pair.first, Constants::FIELD_SIZE - pair.second) += 1.;
            context.getFoodProbItem(Constants::FIELD_SIZE - pair.first, Constants::FIELD_SIZE - pair.second) += 1.;

            context.addFood(pair.first, pair.second);
            context.addFood(Constants::FIELD_SIZE - pair.first, pair.second);
            context.addFood(pair.first, Constants::FIELD_SIZE - pair.second);
            context.addFood(Constants::FIELD_SIZE - pair.first, Constants::FIELD_SIZE - pair.second);
        }
    }

    inline static void updatePP(ExtContext &context) {
        auto f1Ptr = &FoodDetection::foodField;
        auto f2Ptr = &FoodDetection::backupField;
        for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
            for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                if (context.prob[i][j] != 1.) {
                    (*f1Ptr)[i][j] = context.prob[i][j];
                }
                (*f2Ptr)[i][j] = 0.;
            }
        }
        for (int t = 0; t != 20; ++t) {
            for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
                for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                    //
                    for (int x = -1; x != 2; ++x) {
                        int cX = x + i;
                        if (cX < 0 || cX >= Constants::EAT_FIELD_COUNT) {
                            continue;
                        }
                        for (int y = -1; y != 2; ++y) {
                            int cY = y + j;
                            if (cY < 0 || cY >= Constants::EAT_FIELD_COUNT) {
                                continue;
                            }
                            (*f2Ptr)[cX][cY] += (*f1Ptr)[i][j] * diff[x + 1][y + 1];
                        }
                    }
                    (*f1Ptr)[i][j] = 0.;
                }
            }
            auto tmp = f1Ptr;
            f1Ptr = f2Ptr;
            f2Ptr = tmp;
        }
    }

    inline static void updateProbs(ExtContext &extContext) {
        extContext.vision.clear();
        for (auto &playerPart : extContext.me->parts) {
            extContext.vision.emplace_back(playerPart, (int) extContext.me->parts.size());
        }
        std::set<std::pair<int, int> > foodSet;
        for (auto &item : extContext.food) {
            foodSet.insert(std::make_pair((int) (item.x + 0.1), (int) (item.y + 0.1)));
        }

        for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
            for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                bool canSee = false;
                for (auto &vc : extContext.vision) {
                    if (vc.canSee(i, j)) {
                        extContext.prob[i][j] = 0;
                        canSee = true;
                        auto &foodVec = extContext.knownFood[i][j];
                        for (auto it = foodVec.begin(); it != foodVec.end();) {
                            std::pair<int, int> key(it->x, it->y);
                            auto rez = foodSet.find(key);
                            if (rez == foodSet.end()) {
                                it->del();
                                it = foodVec.erase(it);
                            } else {
                                foodSet.erase(rez);
                                ++it;
                            }
                        }
                        break;
                    }
                }
                if (!canSee) {
                    extContext.prob[i][j] *= Constants::FOOD_HIDE_PER_TICK_PROB;
                }
                seenThisTick[i][j] = canSee;
            }
        }
        std::vector<ViewCircle> vcVector;
        for (auto &player : extContext.players) {
            if (&player == extContext.me) {
                continue;
            }
            for (auto &playerPart : player.parts) {
                vcVector.emplace_back(playerPart, player.parts.size() > 3 ? player.parts.size() : 1);
            }
        }

        for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
            for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                for (auto &vc : vcVector) {
                    if (vc.canSee(i, j)) {
                        extContext.prob[i][j] *= .5;
                        break;
                    }
                }
            }
        }
        for (auto &food : extContext.food) {
            if (!extContext.foodExist(food.getX(), food.getY())) { // hm... we need to add this set.
                extContext.addFood((int) food.getX(), (int) (food.getY()));
                extContext.addFood((int) (Constants::FIELD_SIZE - food.getX()), (int) (food.getY()));
                extContext.addFood((int) (food.getX()), (int) (Constants::FIELD_SIZE - food.getY()));
                extContext.addFood((int) (Constants::FIELD_SIZE - food.getX()), (int) (Constants::FIELD_SIZE - food.getY()));
                int x, y;
                ExtContext::getSquare(food.getX(), food.getY(), x, y);

                std::vector<int> ticks;
                ticks.reserve(4);
                ticks.push_back(extContext.lastSeenTick[x][y]);
                ticks.push_back(extContext.lastSeenTick[Constants::EAT_FIELD_COUNT - x - 1][y]);
                ticks.push_back(extContext.lastSeenTick[x][Constants::EAT_FIELD_COUNT - y - 1]);
                ticks.push_back(extContext.lastSeenTick[Constants::EAT_FIELD_COUNT - x - 1][Constants::EAT_FIELD_COUNT - y - 1]);
                std::sort(ticks.begin(), ticks.end());
                // ticks[2] - plausible seen tick

                int guessedSpawnTick = (ticks[3] + Constants::ADD_FOOD_DELAY) / Constants::ADD_FOOD_DELAY * Constants::ADD_FOOD_DELAY;
                int ticksElapsed = extContext.getTickIndex() - guessedSpawnTick;
                double newProb = pow(Constants::FOOD_HIDE_PER_TICK_PROB, ticksElapsed);

                extContext.getFoodProbItem(Constants::FIELD_SIZE - food.getX(), food.getY()) += newProb;
                extContext.getFoodProbItem(food.getX(), Constants::FIELD_SIZE - food.getY()) += newProb;
                extContext.getFoodProbItem(Constants::FIELD_SIZE - food.getX(), Constants::FIELD_SIZE - food.getY()) += newProb;
            }
            extContext.getFoodProbItem(food.getX(), food.getY()) = 1.;
        }
        updatePP(extContext);

        for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
            for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                if (seenThisTick[i][j]) {
                    extContext.lastSeenTick[i][j] = extContext.getTickIndex();
                }
            }
        }
    }

    inline static void initPreFoodSpawnTick(ExtContext &extContext) {
        extContext.prevTickFood.clear();
        for (auto &food : extContext.food) {
            extContext.prevTickFood.emplace_back(food.getX(), food.getY());
        }
    }

    inline static void initFoodSpawnTick(ExtContext &context) {
        std::vector<std::pair<int, int>> initialSets;
        for (auto &food : context.food) {
            bool found = false;
            for (auto it = context.prevTickFood.begin(); it != context.prevTickFood.end();) {
                if (it->first == food.getX() && it->second == food.getY()) {
                    context.prevTickFood.erase(it);
                    found = true;
                    break;
                }
                ++it;
            }
            if (!found) {
                // check if we can see it last step
                for (auto &vc : context.vision) {
                    if (vc.canSee(food)) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    auto x = (int) (food.getX() + 0.1);
                    auto y = (int) (food.getY() + 0.1);
                    if (x > Constants::HALF_FIELD) {
                        x = Constants::FIELD_SIZE - x;
                    }
                    if (y > Constants::HALF_FIELD) {
                        y = Constants::FIELD_SIZE - y;
                    }
                    initialSets.emplace_back(x, y);
                }
            }
        }
        std::sort(initialSets.begin(), initialSets.end());
        initialSets.erase(std::unique(initialSets.begin(), initialSets.end()), initialSets.end());
        int elapsedSets = (int) (Constants::ADD_FOOD_SETS - initialSets.size());
        if (elapsedSets != 0) {
            int seenCount[4] = {0, 0, 0, 0};
            for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
                int idx = ((i >= Constants::EAT_FIELD_COUNT / 2) ? 2 : 0);
                for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                    idx |= ((j >= Constants::EAT_FIELD_COUNT / 2) ? 1 : 0);
                    bool canSee = false;
                    for (auto &vc : context.vision) {
                        if (vc.canSee(i, j)) {
                            canSee = true;
                            break;
                        }
                    }
                    if (canSee) {
                        ++seenCount[idx];
                        context.rememberSeen(i, j);
                        context.rememberSeen(i, Constants::EAT_FIELD_COUNT - j - 1);
                        context.rememberSeen(Constants::EAT_FIELD_COUNT - i - 1, j);
                        context.rememberSeen(Constants::EAT_FIELD_COUNT - i - 1, Constants::EAT_FIELD_COUNT - j - 1);
                    }
                }
            }
            int maxSeen = 0;
            for (int i = 0; i != 4; ++i) {
                maxSeen = std::max(seenCount[i], maxSeen);
            }
            int totalNotSeenClear = Constants::TOTAL_ONE_QUARTER_CELLS - maxSeen;
            double prob = ((double) elapsedSets) / totalNotSeenClear;
            for (int i = 0; i != Constants::EAT_FIELD_COUNT; ++i) {
                for (int j = 0; j != Constants::EAT_FIELD_COUNT; ++j) {
                    if (context.prob[i][j] < 0) {
                        context.prob[i][j] += ExtContext::REMEMBER_NEGATION;
                    } else {
                        context.prob[i][j] += prob;
                    }
                }
            }
        }

        for (auto &pair : initialSets) {
            context.getFoodProbItem(pair.first, pair.second) += 1.;
            context.getFoodProbItem(Constants::FIELD_SIZE - pair.first, pair.second) += 1.;
            context.getFoodProbItem(pair.first, Constants::FIELD_SIZE - pair.second) += 1.;
            context.getFoodProbItem(Constants::FIELD_SIZE - pair.first, Constants::FIELD_SIZE - pair.second) += 1.;

            context.addFood(pair.first, pair.second);
            context.addFood(Constants::FIELD_SIZE - pair.first, pair.second);
            context.addFood(pair.first, Constants::FIELD_SIZE - pair.second);
            context.addFood(Constants::FIELD_SIZE - pair.first, Constants::FIELD_SIZE - pair.second);
        }
    }
};


#endif //ALMOSTAGARIO_FOODDETECTION_H
