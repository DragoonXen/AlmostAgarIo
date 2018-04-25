//
// Created by dragoon on 4/10/18.
//

#include "FoodDetection.h"

double FoodDetection::foodField[Constants::EAT_FIELD_COUNT][Constants::EAT_FIELD_COUNT];
double FoodDetection::backupField[Constants::EAT_FIELD_COUNT][Constants::EAT_FIELD_COUNT];
bool FoodDetection::seenThisTick[Constants::EAT_FIELD_COUNT][Constants::EAT_FIELD_COUNT];

