//
// Created by dragoon on 3/27/18.
//

#include "Ejection.h"
#include "../Constants.h"

Ejection::Ejection(int id, double x, double y, int playerId, int updateTick) : GameObject(id,
                                                                                          x,
                                                                                          y,
                                                                                          0.,
                                                                                          0.,
                                                                                          Constants::EJECTION_MASS(),
                                                                                          Constants::EJECTION_RADIUS(),
                                                                                          updateTick), playerId(playerId) {}
