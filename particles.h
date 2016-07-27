#ifndef PARTICLES_H
#define PARTICLES_H

#include <cmath>
#include <QList>
#include <QDebug>

class Parts
{
public:
    double x, y;
    double speed;
    double angle;
    int lifetime;
    short red;
    short green;
    short blue;

    Parts() { x = 0; y = 0; speed = 0; angle = 0; lifetime = 0; red = 0; green = 0; blue = 0;}
    Parts(double x, double y, double sp, double an, int lt, short r, short g, short b)
    {
        this->x = x;
        this->y = y;
        speed = sp;
        angle = an;
        lifetime = lt;
        red = r;
        green = g;
        blue = b;
    }

    void update()
    {
        x = x + speed*cos(angle);
        y = y + speed*sin(angle);
        lifetime++;
    }

};

class Particles
{
public:
    int x, y;
    bool spawn;
    int r, g, b;
    QList<Parts> parts;

    Particles();
    void update();

    void setSpawn(int x, int y);
    void deleteSpawn();
};

#endif // PARTICLES_H
