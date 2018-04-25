//
// Created by dragoon on 3/27/18.
//

#ifndef ALMOSTAGARIO_CONSTANTS_H
#define ALMOSTAGARIO_CONSTANTS_H

#include "../nlohmann/json.hpp"

using json = nlohmann::json;

class Constants {

public:
#ifdef TL_PREVENTION_LOCAL
    static constexpr int TL = 10500;
#else
    static constexpr int TL = 20400;
#endif
    static constexpr int SPLIT_DELAY = 5;
    static constexpr int MA_PERIOD = 1000;

    static constexpr int VISION_REMEMBER = 10;

    static constexpr double EATING_RAD = 1. / 3.;
    static constexpr double VIRUS_HURT_RAD = 2. / 3.;

    static constexpr double BURST_BONUS = 5.;
    static constexpr double MIN_BURST_MASS = 60.;
    static constexpr double MIN_SPLIT_MASS = 120.;

    static constexpr double SCORE_FOR_BURST = 2.;
    static constexpr double SCORE_FOR_ENEMY_BURST = -2.;

    static constexpr double BURST_ANGLE_SPECTRUM = M_PI;
    static constexpr double BURST_START_SPEED = 8.;
    static constexpr double SPLIT_START_SPEED = 9.;

    static constexpr double VIS_FACTOR = 4.0;
    static constexpr double VIS_FACTOR_FR = 2.5;
    static constexpr double VIS_SHIFT = 10.0;
    static constexpr double COLLISION_POWER = 20.;
    static constexpr int SHRINK_EVERY_TICK = 50;
    static constexpr double MIN_SHRINK_MASS = 100.;
    static constexpr double SHRINK_FACTOR = 0.01;

    static constexpr double RADIUS_FACTOR = 2.;

    static constexpr double SPEED_SCORE_BONUS = .0001;
    static constexpr double FOOD_PP_SCORE_BONUS = 0.006649;

    static constexpr double FOOD_PROB_STEP = 30.;

    static constexpr double ENEMY_EAT_NEGATION_KOEFF = .5;
    static constexpr double ENEMY_EAT_ME_SCORE = 10.;
//    static constexpr double ENEMY_EAT_ANY_SCORE = 5.;
    static constexpr double ENEMY_DESTROY_ME = 10000.;
    static constexpr double ME_DESTROY_ENEMY = 10.;
    static constexpr double ME_EAT_ENEMY = 10.;

    static constexpr double MY_DANGER_COEFF = 10.;
    static constexpr double MY_DANGER_DESTROY = 10000.;
    static constexpr double ENEMY_DANGER_COEFF = 7.;

    static constexpr double MASS_EAT_FACTOR = 1.2;
    static constexpr double FOOD_RADIUS = 2.5;

    // FOOD REGION
    static constexpr int EAT_FIELD_COUNT = 64; // must divided on 2
    static constexpr int TOTAL_ONE_QUARTER_CELLS = EAT_FIELD_COUNT * EAT_FIELD_COUNT / 4;
    static constexpr double COORDS_IN_ONE_FOOD_FIELD = 990. / EAT_FIELD_COUNT; // hm...
    static constexpr int HALF_FIELD = 990 / 2; // hm...
    static constexpr int FIELD_SIZE = 990;
    static constexpr double HALF_COORDS_IN_ONE_FOOD_FIELD = COORDS_IN_ONE_FOOD_FIELD / 2.;

    static constexpr double ADD_SCORE_FOR_PREV = 1e-5;

    static constexpr int START_FOOD_SETS = 4;
    static constexpr int ADD_FOOD_SETS = 2;
    static constexpr int ADD_FOOD_DELAY = 40;

    static constexpr double FOOD_HIDE_PER_TICK_PROB = .99;

    static const double ENOUGHT_SEEN_DISTANCE;

    // END FOOD DETECTION REGION

private:
    static constexpr int MAX_MULT_INDEX = 100;
    double ejectionRadius = 4.0;
    double ejectionMass = 15.0;
    int myPlayerId;

    double foodMass, virusRadius, virusSplitMass, viscosity, inertionFactor, speedFactor;
    int maxFragsCnt, ticksTilFusion;

    int gameWidth, gameHeight, gameTicks;
    int halfWidth, halfHeight;

    static std::unique_ptr<Constants> instance;

public:
    static constexpr int DANGER_FIELD_SIZE = 990;
    static double FIELD_DANGER[DANGER_FIELD_SIZE][DANGER_FIELD_SIZE];
    static constexpr double SAFE_RADIUS = 600.;
    static constexpr double POS_DANGER_MULT = 3.;

    static constexpr double PREV_TICK_SCORE_MULT = 1.00001;
    static double INDEX_MULT[MAX_MULT_INDEX];

    static void initConstants(const json &json);

private:
    Constants(const json &json);

public:
    static double EJECTION_RADIUS();

    static double EJECTION_MASS();

    static double FOOD_MASS();

    static int GAME_WIDTH();

    static int GAME_HEIGHT();

    static int HALF_WIDTH();

    static int HALF_HEIGHT();

    static int GAME_TICKS();

    static double VIRUS_RADIUS();

    static double VIRUS_SPLIT_MASS();

    static double VISCOSITY();

    static double INERTION_FACTOR();

    static double SPEED_FACTOR();

    static int MAX_FRAGS_CNT();

    static int TICKS_TIL_FUSION();

    static void setMyId(int myId);
};

#endif //ALMOSTAGARIO_CONSTANTS_H
