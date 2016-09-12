#include "life.h"

Life::Life()
{
    hold = false;

    map = new short*[350];
    nextMap = new short*[350];
    colorRed = new short*[350];
    colorGreen = new short*[350];
    colorBlue = new short*[350];
    nextColorRed = new short*[350];
    nextColorGreen = new short*[350];
    nextColorBlue = new short*[350];
    for (int i = 0; i < 350; i++)
    {
        map[i] = new short[350];
        nextMap[i] = new short[350];
        colorRed[i] = new short[350];
        colorGreen[i] = new short[350];
        colorBlue[i] = new short[350];
        nextColorRed[i] = new short[350];
        nextColorGreen[i] = new short[350];
        nextColorBlue[i] = new short[350];
        for (int j = 0; j < 350; j++)
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
    {
        delete colorRed;
        delete colorGreen;
        delete colorBlue;
        delete nextColorRed;
        delete nextColorGreen;
        delete nextColorBlue;
    }
}

void Life::initialize(int wi, int he)
{
    //Life(wi, he);

    srand((int)map);

    int count = 3;
    while (count > 0)
    {
        int x = rand()%350;
        int y = rand()%350;
        int radius = 20+rand()%30;
        int r,g,b;
        r = 80+rand()%160;
        g = 80+rand()%120;
        b = 80+rand()%160;

        int difx;
        int dify;
        int far;
        for (int i = x-radius; i < x+radius; i++)
        {
            for (int j = y-radius; j < y+radius; j++)
            {
                if (i < 0 || i >= 350 || j < 0 || j >= 350) continue;

                difx = x - i;
                dify = y - j;
                far = sqrt(difx*difx+dify*dify);
                if (far > radius) continue;

                if (rand()%(far/3+2) == 0)
                {
                    map[i][j] = 255;
                    colorRed[i][j] = r;
                    colorGreen[i][j] = g;
                    colorBlue[i][j] = b;
                    // убрать, если нужны "плавные" цвета
                    nextColorRed[i][j] = r;
                    nextColorGreen[i][j] = g;
                    nextColorBlue[i][j] = b;
                }
            }
        }


        count--;
    }
}

void Life::update()
{
    size_t size = sizeof(short)*350;
    int neighboor = 0;
    int avR = 0, avG = 0, avB = 0;
    for (int i = 0; i < 350; i++)
    {
        for (int j = 0; j < 350; j++)
        {
            neighboor = avR = avG = avB = 0;
            for (int m = i-1; m <= i+1; m++)
            {
                for (int n = j-1; n <= j+1; n++)
                {
                    if (m < 0 || m >= 350 || n < 0 || n >= 350 || (m == i && n == j))
                        continue;

                    if (map[m][n] > 0)
                    {
                        neighboor++;
                        avR += colorRed[m][n];
                        avG += colorGreen[m][n];
                        avB += colorBlue[m][n];
                    }
                }
            }


            if (neighboor < 2 || neighboor > 3)
            {
                nextMap[i][j] = 0;
                continue;
            }

            if ((neighboor == 2 || neighboor == 3) && map[i][j] > 0)
            {
                /*nextColorRed[i][j] = colorRed[i][j];
                nextColorGreen[i][j] = colorGreen[i][j];
                nextColorBlue[i][j] = colorBlue[i][j];*/
                nextMap[i][j] = map[i][j] - 1;
                continue;
            }

            if (neighboor == 3 && map[i][j] == 0)
            {
                /*avR = avR/neighboor;
                avG = avG/neighboor;
                avB = avB/neighboor;*/
                /*if (avR < 50) avR = 50;
                if (avG < 50) avG = 50;
                if (avB < 50) avB = 50;*/


                nextMap[i][j] = 255;
                nextColorRed[i][j] = avR/3;
                nextColorGreen[i][j] = avG/3;
                nextColorBlue[i][j] = avB/3;
                continue;
            }

            if (neighboor > 0 && neighboor < 5)
            {
                if (rand()%1000 == 0)
                {
                    for (int m = i-1; m <= i+1; m++)
                    {
                        for (int n = j-1; n <= j+1; n++)
                        {
                            if (m < 0 || m >= 350 || n < 0 || n >= 350 || (m == i && n == j))
                                continue;
                            if (map[m][n] > 0)
                            {
                                nextColorRed[i][j] = colorRed[m][n];
                                nextColorGreen[i][j] = colorGreen[m][n];
                                nextColorBlue[i][j] = colorBlue[m][n];
                                break;
                            }
                        }
                    }
                    nextMap[i][j] = 255;
                }
            }


        }

        if (i >= 5)
        {
            memcpy(map[i-5],nextMap[i-5],size);
            memcpy(colorRed[i-5],nextColorRed[i-5],size);
            memcpy(colorGreen[i-5],nextColorGreen[i-5],size);
            memcpy(colorBlue[i-5],nextColorBlue[i-5],size);
        }
    }



    for (int i = 345; i < 350; i++)
    {
        memcpy(map[i],nextMap[i],size);
        memcpy(colorRed[i],nextColorRed[i],size);
        memcpy(colorGreen[i],nextColorGreen[i],size);
        memcpy(colorBlue[i],nextColorBlue[i],size);
    }

    /*for (int i = 0; i < 350; i++)
    {
        for (int j = 0; j < 350; j++)
        {
            map[i][j] = nextMap[i][j];
            colorRed[i][j] = nextColorRed[i][j];
            colorGreen[i][j] = nextColorGreen[i][j];
            colorBlue[i][j] = nextColorBlue[i][j];
        }
    }*/
}

void Life::spawn(int x, int y)
{
    if (!hold)
    {
        x = x/2;
        y = y/2;
        int radius = 30+rand()%50;
        int r,g,b;
        r = 80+rand()%160;
        g = 80+rand()%100;
        b = 80+rand()%160;
        /*for (int i = x-radius; i < x+radius; i++)
        {
            for (int j = y-(radius-abs(x-i)); j < y+(radius-abs(x-i)); j++)
            {
                if (rand()%4 == 0)
                {

                    if (i < 0 || i >= 350 || j < 0 || j >= 350) continue;
                    map[i][j] = 255;
                    colorRed[i][j] = r;
                    colorGreen[i][j] = g;
                    colorBlue[i][j] = b;
                }
            }
        }*/
        int difx;
        int dify;
        int far;
        for (int i = x-radius; i < x+radius; i++)
        {
            for (int j = y-radius; j < y+radius; j++)
            {
                if (i < 0 || i >= 350 || j < 0 || j >= 350) continue;

                difx = x - i;
                dify = y - j;
                far = sqrt(difx*difx+dify*dify);
                if (far > radius) continue;

                if (rand()%(far/3+2) == 0)
                {
                    map[i][j] = 255;
                    colorRed[i][j] = r;
                    colorGreen[i][j] = g;
                    colorBlue[i][j] = b;
                    nextColorRed[i][j] = r;
                    nextColorGreen[i][j] = g;
                    nextColorBlue[i][j] = b;
                }
            }
        }
    }
    hold = true;
}
