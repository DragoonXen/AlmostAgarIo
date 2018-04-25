//
// Created by dragoon on 3/27/18.
//

#ifndef ALMOSTAGARIO_SOLVER_H
#define ALMOSTAGARIO_SOLVER_H


#include "Constants.h"
#include "../nlohmann/json.hpp"
#include "models/Command.h"
#include "ExtContext.h"

class Solver {

public:
    Command onTick(ExtContext &context, json &input);

//    template<class T>
//    json find_food(const T &objects);
};


#endif //ALMOSTAGARIO_SOLVER_H
