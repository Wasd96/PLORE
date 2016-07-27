#include "particles.h"


Particles::Particles()
{
    spawn = false;
    x = 0; y = 0;
    r = 0; g = 0; b = 0;
}

void Particles::update()
{
    for(int i = 0; i < parts.size(); i++)
    {
        parts[i].update();
        if (rand()%(100-parts.at(i).lifetime) == 0)
        {
            parts.removeAt(i);
        }

    }

    Parts p;
    if (spawn)
    {
        p.angle = (rand()%360)/180.0*M_PI;
        p.speed = 2+(rand()%20)/10.0;
        p.x = x;
        p.y = y;
        p.red = r + rand()%40-20;
        p.green = g + rand()%40-20;
        p.blue = b + rand()%40-20;

        parts.append(p);
    }
}

void Particles::setSpawn(int x, int y)
{
    if (!spawn)
    {
        spawn = true;
        this->x = x;
        this->y = y;
        r = 100 + rand()%130;
        g = 100 + rand()%130;
        b = 100 + rand()%130;
    }
}

void Particles::deleteSpawn()
{
    spawn = false;
}

