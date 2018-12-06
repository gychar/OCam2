#ifndef MEDIAN_H
#define MEDIAN_H

#include <cmath>
#include <vector>
#include <numeric>
#include <iterator>
#include <iostream>
#include <algorithm>

using namespace std;

class Maths
{
public:
    Maths();
    float Median(vector<float> const& v);
    float Mean(vector<float> a);
    float Variance(vector<float> a);
    float StandardDeviation(vector<float> a);
};

#endif // MEDIAN_H
