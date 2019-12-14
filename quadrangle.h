#pragma once
#include <opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;
/*
1   3

2   4
*/
class quadrangle
{
public:
    vector<Point> points;
    quadrangle(vector<Point> src)
    {
        assert(src.size() == 4);

        for (int i = 0; i < 2; i++)
        {
            for (int j = 2; j < 4; j++)
            {
                if (src[i].x > src[j].x)
                {
                    Point swap = src[i];
                    src[i] = src[j];
                    src[j] = swap;
                }
            }
        }

        for (int i = 0; i < 4; i+=2)
        {
            if (src[i].y > src[i+1].y)
            {
                Point swap = src[i];
                src[i] = src[i+1];
                src[i+1] = swap;
            }
            
        }

        points = src;
    }
};

