//
// Created by dragoon on 3/27/18.
//

#include "Constants.h"
#include "Utils.h"

std::unique_ptr<Constants> Constants::instance;

double Constants::FIELD_DANGER[DANGER_FIELD_SIZE][DANGER_FIELD_SIZE];
double Constants::INDEX_MULT[MAX_MULT_INDEX];

void Constants::initConstants(const json &json) {
    instance = std::make_unique<Constants>(Constants(json));
    INDEX_MULT[0] = 1.;
    for (int i = 1; i != MAX_MULT_INDEX; ++i) {
        INDEX_MULT[i] = INDEX_MULT[i - 1] * PREV_TICK_SCORE_MULT;
    }
    double midX = Constants::GAME_WIDTH() / 2.;
    double midY = Constants::GAME_HEIGHT() / 2.;
    double safeSqrRadius = Constants::SAFE_RADIUS;
    safeSqrRadius *= safeSqrRadius;

    double halfPoint = Constants::GAME_HEIGHT() / (DANGER_FIELD_SIZE * 2.);
    for (int i = 0; i != DANGER_FIELD_SIZE; ++i) {
        double x = (i * 2 + 1) * halfPoint;
        for (int j = 0; j != DANGER_FIELD_SIZE; ++j) {
            double y = (j * 2 + 1) * halfPoint;
            double sqrDist = Utils::distSqr(x, y, midX, midY);
            sqrDist -= safeSqrRadius;
            if (sqrDist > 0.) {
                FIELD_DANGER[i][j] = (sqrDist / safeSqrRadius) * POS_DANGER_MULT;
            } else {
                FIELD_DANGER[i][j] = 0.;
            }
        }
    }
}

Constants::Constants(const json &json) {
    this->gameTicks = json["GAME_TICKS"];
    this->gameWidth = json["GAME_WIDTH"];
    this->gameHeight = json["GAME_HEIGHT"];
    this->foodMass = json["FOOD_MASS"];
    this->maxFragsCnt = json["MAX_FRAGS_CNT"];
    this->ticksTilFusion = json["TICKS_TIL_FUSION"];
    this->virusRadius = json["VIRUS_RADIUS"];
    this->virusSplitMass = json["VIRUS_SPLIT_MASS"];
    this->viscosity = json["VISCOSITY"];
    this->inertionFactor = json["INERTION_FACTOR"];
    this->speedFactor = json["SPEED_FACTOR"];
    this->halfWidth = this->gameWidth / 2;
    this->halfHeight = this->gameHeight / 2;
}

double Constants::EJECTION_RADIUS() {
    return instance->ejectionRadius;
}

double Constants::EJECTION_MASS() {
    return instance->ejectionMass;
}

double Constants::FOOD_MASS() {
    return instance->foodMass;
}

int Constants::GAME_WIDTH() {
    return instance->gameWidth;
}

int Constants::GAME_HEIGHT() {
    return instance->gameHeight;
}

int Constants::GAME_TICKS() {
    return instance->gameTicks;
}

double Constants::VIRUS_RADIUS() {
    return instance->virusRadius;
}

double Constants::VIRUS_SPLIT_MASS() {
    return instance->virusSplitMass;
}

double Constants::VISCOSITY() {
    return instance->viscosity;
}

double Constants::INERTION_FACTOR() {
    return instance->inertionFactor;
}

double Constants::SPEED_FACTOR() {
    return instance->speedFactor;
}

int Constants::MAX_FRAGS_CNT() {
    return instance->maxFragsCnt;
}

int Constants::TICKS_TIL_FUSION() {
    return instance->ticksTilFusion;
}

int Constants::HALF_WIDTH() {
    return instance->halfWidth;
}

int Constants::HALF_HEIGHT() {
    return instance->halfHeight;
}

void Constants::setMyId(int myId) {
    instance->myPlayerId = myId;
}

const double Constants::ENOUGHT_SEEN_DISTANCE = sqrt(HALF_COORDS_IN_ONE_FOOD_FIELD * HALF_COORDS_IN_ONE_FOOD_FIELD * 2) - Constants::FOOD_RADIUS;