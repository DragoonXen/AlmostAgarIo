//
// Created by dragoon on 3/27/18.
//

#include "GameObject.h"

int GameObject::getId() const {
    return id;
}

double GameObject::getX() const {
    return x;
}

double GameObject::getY() const {
    return y;
}

double GameObject::getDx() const {
    return dx;
}

double GameObject::getDy() const {
    return dy;
}

int GameObject::getUpdateTick() const {
    return updateTick;
}

GameObject::GameObject(int id, double x, double y, double sx, double sy, double m, double r, int updateTick)
        : id(id), x(x), y(y), dx(sx), dy(sy), mass(m), radius(r), updateTick(updateTick) {}
