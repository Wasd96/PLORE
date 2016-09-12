#ifndef LIFE_H
#define LIFE_H

#include "stdlib.h"
#include "cmath"
#include <QDebug>

class Life
{
private:
    bool hold;

public:

    short **colorRed;
    short **colorGreen;
    short **colorBlue;
    short **nextColorRed;
    short **nextColorGreen;
    short **nextColorBlue;
    short **map;
    short **nextMap;

public:
    Life();
    ~Life();

    void update();
    void release() { hold = false; }
    void spawn(int x, int y);

    void initialize(int wi, int he);

};

#endif // LIFE_H
