#include "life.h"


Life::Life()
{
    hold = false;

    map = new int*[100];
    nextMap = new int*[100];
    colorRed = new int*[100];
    colorGreen = new int*[100];
    colorBlue = new int*[100];
    nextColorRed = new int*[100];
    nextColorGreen = new int*[100];
    nextColorBlue = new int*[100];
    for (int i = 0; i < 100; i++)
    {
        map[i] = new int[100];
        nextMap[i] = new int[100];
        colorRed[i] = new int[100];
        colorGreen[i] = new int[100];
        colorBlue[i] = new int[100];
        nextColorRed[i] = new int[100];
        nextColorGreen[i] = new int[100];
        nextColorBlue[i] = new int[100];
        for (int j = 0; j < 100; j++)
        {
            map[i][j] = 0;
            nextMap[i][j] = 0;
            colorRed[i][j] = 0;
            colorGreen[i][j] = 0;
            colorBlue[i][j] = 0;
            nextColorRed[i][j] = 0;
            nextColorGreen[i][j] = 0;
            nextColorBlue[i][j] = 0;
        }
    }
}

Life::~Life()
{
    if (map != NULL)
        delete map;
    if (nextMap != NULL)
        delete nextMap;
    if (colorRed != NULL)
        delete colorRed, colorGreen, colorBlue;
}

void Life::initialize()
{
    srand((int)map);

    int count = 3+rand()%3;
    while (count > 0)
    {
        int x = rand()%100;
        int y = rand()%100;
        int radius = 10+rand()%20;
        int r,g,b;
        r = 50+rand()%50;
        g = 50+rand()%50;
        b = 50+rand()%50;
        for (int i = x-radius; i < x+radius; i++)
        {
            for (int j = y-(radius-abs(x-i)); j < y+(radius-abs(x-i)); j++)
            {
                if (rand()%5 == 0)
                {

                    if (i < 0 || i >= 100 || j < 0 || j >= 100) continue;
                    map[i][j] = 255;
                    colorRed[i][j] = r;
                    colorGreen[i][j] = g;
                    colorBlue[i][j] = b;
                }
            }
        }


        count--;
    }
}

void Life::update()
{
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            int neighboor = 0;
            int avR = 0, avG = 0, avB = 0;
            for (int m = i-1; m <= i+1; m++)
            {
                for (int n = j-1; n <= j+1; n++)
                {
                    if (m < 0 || m >= 100 || n < 0 || n >= 100)
                        continue;
                    if (map[m][n] > 0)
                    {
                        neighboor++;
                        if (neighboor < 3)
                        avR += colorRed[m][n];
                        avG += colorGreen[m][n];
                        avB += colorBlue[m][n];
                    }
                }
            }


            if (neighboor < 2 || neighboor > 3)
            {
                nextMap[i][j] = 0;
            }

            if ((neighboor == 2 || neighboor == 3) && map[i][j] > 0)
                nextMap[i][j] = map[i][j] - 1;

            if (neighboor == 3 && map[i][j] == 0)
            {
                avR = avR/neighboor;
                avG = avG/neighboor;
                avB = avB/neighboor;
                if (avR < 255) avR += 2;
                if (avG < 255) avG += 2;
                if (avB < 255) avB += 2;


                nextMap[i][j] = 255;
                nextColorRed[i][j] = avR;
                nextColorGreen[i][j] = avG;
                nextColorBlue[i][j] = avB;
            }
        }
    }


    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            map[i][j] = nextMap[i][j];
            colorRed[i][j] = nextColorRed[i][j];
            colorGreen[i][j] = nextColorGreen[i][j];
            colorBlue[i][j] = nextColorBlue[i][j];
        }
    }
}

void Life::spawn(int x, int y)
{
    if (!hold)
    {
        x = x/7;
        y = y/7;
        int radius = 10+rand()%10;
        int r,g,b;
        r = 50+rand()%100;
        g = 50+rand()%100;
        b = 50+rand()%100;
        for (int i = x-radius; i < x+radius; i++)
        {
            for (int j = y-(radius-abs(x-i)); j < y+(radius-abs(x-i)); j++)
            {
                if (rand()%5 == 0)
                {

                    if (i < 0 || i >= 100 || j < 0 || j >= 100) continue;
                    map[i][j] = 255;
                    colorRed[i][j] = r;
                    colorGreen[i][j] = g;
                    colorBlue[i][j] = b;
                }
            }
        }
    }
    hold = true;
}
