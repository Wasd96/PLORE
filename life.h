#ifndef LIFE_H
#define LIFE_H

#include "stdlib.h"
#include <QDebug>

class Life
{
private:
    bool hold;

public:
    int **colorRed;
    int **colorGreen;
    int **colorBlue;
    int **nextColorRed;
    int **nextColorGreen;
    int **nextColorBlue;
    int **map;
    int **nextMap;

public:
    Life();
    ~Life();

    void update();
    void release() { hold = false; }
    void spawn(int x, int y);

    void initialize();


};

#endif // LIFE_H
