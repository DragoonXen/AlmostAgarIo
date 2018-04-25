//
// Created by dragoon on 4/1/18.
//

#include "PlayerPartSim.h"
#include "../../Utils.h"



PlayerPartSim::PlayerPartSim(const PlayerPart &copyFrom) : PlayerPart(copyFrom.getId(),
                                                                      copyFrom.getPlayerId(),
                                                                      copyFrom.getX(),
                                                                      copyFrom.getY(),
                                                                      copyFrom.getDx(),
                                                                      copyFrom.getDy(),
                                                                      copyFrom.getMass(),
                                                                      copyFrom.getRadius(),
                                                                      copyFrom.getUpdateTick(),
                                                                      copyFrom.getTtf()) {

}